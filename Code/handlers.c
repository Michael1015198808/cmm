#include "common.h"
#include "type.h"
#include "error.h"
#include "handlers.h"
#include "table.h"

make_handler(semantic) { // Most top-level handler. Default handler
    if(cur -> func) {
        return cur -> func(cur);
    } else {
        for(int i = 0; i < cur->cnt; ++i) {
            if(cur->siblings[i])
                semantic_handler(cur->siblings[i]);
        }
        return NULL;
    }
}

char* get_vardec_name(node* cur) {
    do {
        cur = cur -> siblings[0];
    } while(!strcmp(cur -> name, "VarDec"));
    return cur -> val_str;
}

Type cur_def_type = NULL;
make_handler(def) {
    Type old_def_type = cur_def_type;
    cur_def_type = cur -> siblings[0] -> func(cur -> siblings[0]);
    if(cur -> cnt == 3) {
        semantic_handler(cur -> siblings[1]);
    }
    cur_def_type = old_def_type;
    return NULL;
}

make_handler(variable) {// cur : VarDec -> ID
    return cur_def_type;
}

make_handler(array_dec) {// cur : VarDec -> VarDec LB INT RB
    node* vardec = cur -> siblings[0];
    Type t = vardec -> func(vardec);
    return to_array(t, cur -> siblings[2] -> val_int);
}

make_handler(extdeclist) {
    node* vardec = cur -> siblings[0];
    const char* const name = get_vardec_name(vardec);
    if(table_insert(name, vardec -> func(vardec))) {
        semantic_error(cur -> lineno, REDEFINE_VARIABLE, name);
    }
    return cur -> siblings[2] -> func(cur -> siblings[2]);
}

make_handler(vardec) {// cur : Dec -> VarDec
                      // cur : ExtDecList -> VarDec
                      // cur : Dec -> VarDec ASSIGNOP Exp
    node* vardec = cur -> siblings[0];
    const char* const name = get_vardec_name(vardec);
    Type t = vardec -> func(vardec);
    if(table_insert(name, t)) {
        semantic_error(cur -> lineno, REDEFINE_VARIABLE, name);
    }
    if(cur -> cnt == 3) {
        node* exp = cur -> siblings[2];
        type_check(t, exp -> func(exp), NULL, exp -> lineno, ASSIGN_MISMATCH);
    }
    return NULL;
}

make_handler(type) {
    switch(cur -> siblings[0] -> val_int) {
        case T_FLOAT:
            return type_float;
        case T_INT:
            return type_int;
        default:
            panic();
            return NULL;
    }
}

static inline Type deflist_to_struct_type(node* cur) {
    Type ret = new(struct Type_);
    struct FieldList_ guard;
    FieldList last = &guard;
    for(;cur && cur -> cnt == 2; cur = cur -> siblings[1]) {
        node* def = cur -> siblings[0];
        node* specifier = def -> siblings[0];
        Type t = specifier -> func(specifier);
        for(node* declist = def -> siblings[1];;) {
            node* vardec = declist -> siblings[0] -> siblings[0];
            last -> next = new(struct FieldList_);
            last = last -> next;
            cur_def_type = t;
            last -> type = vardec -> func(vardec);
            last -> name = get_vardec_name(vardec);
            if(declist -> siblings[0] -> cnt == 3) {
                semantic_error(vardec -> siblings[0] -> lineno, REDEFINE_FIELD, last -> name);
            }
            if(table_insert(last -> name, last -> type)) {
                semantic_error(vardec -> siblings[0] -> lineno, REDEFINE_FIELD, last -> name);
            }
            if(declist -> cnt == 1) {
                break;
            } else {
                declist = declist -> siblings[2];
            }
        }
    }
    last -> next = NULL;
    ret -> kind = STRUCTURE;
    ret -> structure = guard.next;
    return ret;
}

make_handler(struct_specifier) {
    cur = cur -> siblings[0];//Specifier to StructSpecifier
    Type ret = NULL;
    switch(cur -> cnt) {
        case 2:
            ret = table_lookup_struct(cur -> siblings[1] -> siblings[0] -> val_str);
            if(!ret) {
                semantic_error(cur -> lineno, UNDEFINE_STRUCTURE, cur -> siblings[1] -> siblings[0] -> val_str);
            } else {
                ret = ret -> variable;
            }
            break;
        case 5:
            ret = new(struct Type_);
            ret -> kind = STRUCTURE_DEF;
            new_scope();
            ret -> variable = deflist_to_struct_type(cur -> siblings[3]);
            free_scope();
            if(cur -> siblings[1]) {
                if(table_insert_struct(cur -> siblings[1] -> siblings[0] -> val_str, ret)) {
                    semantic_error(cur -> lineno, DUPLICATE_NAME, cur -> siblings[1] -> siblings[0] -> val_str);
                }
            } else {
                add_anonymous_struct(ret);
            }
            ret = ret -> variable;
            break;
        default:
            panic();
    }
    return ret;
}


static Type fun_parsing(node* cur, int is_dec) {
    Type ret = new(struct Type_);
    ret -> kind = FUNCTION;
    ret -> is_dec = is_dec;
    ret -> structure = new(struct FieldList_);
    ret -> structure -> name = "";
    ret -> structure -> type = cur -> siblings[0] -> func(cur -> siblings[0]);

    FieldList last = ret -> structure;
    if(cur -> siblings[1] -> cnt == 4) {
        for(node* varlist = cur -> siblings[1] -> siblings[2]; ;varlist = varlist -> siblings[2]) {
            node* paramdec = varlist -> siblings[0];
            cur_def_type = paramdec -> siblings[0] -> func(paramdec -> siblings[0]);

            last -> next = new(struct FieldList_);
            last = last -> next;
            last -> name = get_vardec_name(paramdec -> siblings[1]);
            last -> type = paramdec -> siblings[1] -> func(paramdec -> siblings[1]);
            if(table_insert(last -> name, last -> type)) {
                semantic_error(paramdec -> lineno, REDEFINE_VARIABLE, last -> name);
            };
            if(varlist -> cnt == 1) {
                break;
            }
        }
    }
    last -> next = NULL;
    return ret;
}

struct fun_dec_list {
    int lineno;
    const char* name;
    Type t;
    struct fun_dec_list* next;
};
static struct fun_dec_list* head = NULL;

void fun_dec_checker() {
    for(struct fun_dec_list* cur = head; cur; cur = remove_access(cur, next)) {
        if(cur -> t -> is_dec) {
            semantic_error(cur -> lineno, DEC_UNDEFINE_FUNCTION, cur -> name);
        }
    }
    head = NULL;
}

make_handler(fun_dec) { // cur : ExtDef -> Specifier FunDec CompSt
    new_scope();
    Type t = fun_parsing(cur, 1);
    free_scope();
    const char *const name = cur -> siblings[1] -> siblings[0] -> val_str;
    const int lineno = cur -> siblings[1] -> siblings[0] -> lineno;
    if(table_insert_global(name, t)) {
        Type old = table_lookup(name);
        if(typecmp(old, t)) {
            semantic_error(lineno, INCONSIS_DEC, name);
        }
    } else {
        struct fun_dec_list* tmp = new(struct fun_dec_list);
        tmp -> lineno = lineno;
        tmp -> name = name;
        tmp -> t = t;
        tmp -> next = head;
        head = tmp;
    }
    return t;
}

static void* compst_without_scope(node* );
static Type cur_fun_ret_type = NULL;
make_handler(fun_def) { // cur : ExtDef -> Specifier FunDec CompSt
    new_scope();
    Type t = fun_parsing(cur, 0);
    if(table_insert_global(cur -> siblings[1] -> siblings[0] -> val_str, t)) {
        Type old = table_lookup(cur -> siblings[1] -> siblings[0] -> val_str);
        if(old -> is_dec) {
            old -> is_dec = 0;
            if(typecmp(old, t)) {
                semantic_error(cur -> siblings[1] -> siblings[0] -> lineno, INCONSIS_DEC, cur -> siblings[1] -> siblings[0] -> val_str);
            }
        } else {
            semantic_error(cur -> siblings[1] -> siblings[0] -> lineno, REDEFINE_FUNCTION, cur -> siblings[1] -> siblings[0] -> val_str);
        }
    }
    Type old_fun_ret_type = cur_fun_ret_type;
    cur_fun_ret_type = t -> structure -> type;
    compst_without_scope(cur -> siblings[2]);
    free_scope();
    cur_fun_ret_type = old_fun_ret_type;
    return NULL;
}

make_handler(return) {// cur : RETURN Exp SEMI
    Assert(cur_fun_ret_type);
    node* exp = cur -> siblings[1];
    type_check(cur_fun_ret_type, exp -> func(exp), NULL, exp -> lineno, RETURN_MISMATCH);
    return NULL;
}

int is_lvalue(node* cur) {
    switch(cur -> cnt) {
        case 1:
            return cur -> func == id_handler;
        case 3:
            //Exp DOT ID
            return cur -> func == struct_access_handler;
        case 4:
            return cur -> func == array_access_handler;
        default:
            return 0;
    }
}

make_handler(assign) { //cur : Exp ASSIGNOP Exp
    Type lhs = cur -> siblings[0] -> func(cur -> siblings[0]);
    Type rhs = cur -> siblings[2] -> func(cur -> siblings[2]);
    if(!is_lvalue(cur -> siblings[0])) {
        semantic_error(cur -> siblings[0] -> lineno, LVALUE);
    }
    return type_check(lhs, rhs, NULL, cur -> siblings[0] -> lineno, ASSIGN_MISMATCH);
}

make_handler(id) { //cur : ID
    cur = cur -> siblings[0];
    Type t = table_lookup(cur -> val_str);
    if(!t) {
        semantic_error(cur -> lineno, UNDEFINE_VARIABLE, cur -> val_str);
    }
    return t;
}

make_handler(binary_op) {
    Type lhs = cur -> siblings[0] -> func(cur -> siblings[0]);
    Type rhs = cur -> siblings[2] -> func(cur -> siblings[2]);
    return type_check(lhs, rhs, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH);
}

make_handler(int) {
    return type_int;
}

make_handler(float) {
    return type_float;
}

make_handler(fun_call) {// cur : ID LP Args RP 
                        //     | ID LP RP
    node* fun = cur -> siblings[0];
    Type func = table_lookup(fun -> val_str);

    if(!func) {
        semantic_error(fun -> lineno, UNDEFINE_FUNCTION, fun -> val_str);
        return NULL;
    }
    if(func -> kind != FUNCTION) {
        semantic_error(fun -> lineno,      NOT_FUNCTION, fun -> val_str);
        return NULL;
    }
    if(cur -> cnt == 4) {
        FieldList param = func -> structure -> next;
        node* args = cur -> siblings[2];
        for(; param;) {
            node* exp = args -> siblings[0];
            Type t = exp -> func(exp);
            if(typecmp(t, param -> type)) {
                break;
            }
            param = param -> next;
            if(args -> cnt == 1) {
                args = NULL;
                break;
            } else {
                args = args -> siblings[2];
            }
        }
        if(param || args) {
            semantic_error(fun -> lineno, FUNCTION_MISMATCH, fun -> val_str);
        }
    } else {
        if(func -> structure -> next) {
            semantic_error(fun -> lineno, FUNCTION_MISMATCH, fun -> val_str);
        }
    }
    return func -> structure -> type;
}

make_handler(array_access) {// cur : Exp LB Exp RB
    node* exp1 = cur -> siblings[0];
    Type base = exp1 -> func(exp1);
    if(base -> kind != ARRAY) {
        semantic_error(exp1 -> lineno, NOT_ARRAY);
        return NULL;
    }
    node* exp2 = cur -> siblings[2];
    Type index = exp2 -> func(exp2);
    return type_check(type_int, index, base -> array.elem, exp2 -> lineno, NOT_INT);
}

make_handler(struct_access) {// cur : Exp DOT ID
    node* exp = cur -> siblings[0];
    Type t = exp -> func(exp);
    if(t) {
        if(t -> kind != STRUCTURE) {
            semantic_error(exp -> lineno, ILLEGAL_USE, ".");
            return NULL;
        }
        for(FieldList f = t -> structure;f;f = f -> next) {
            if(!strcmp(f -> name, cur -> siblings[2] -> val_str)) {
                return f -> type;
            }
        }
        semantic_error(exp -> lineno, NONEXIST_FIELD, cur -> siblings[2] -> val_str);
    }
    return NULL;
}

static void* compst_without_scope(node* cur) {
    if(cur -> siblings[1])
        semantic_handler(cur -> siblings[1]);
    if(cur -> siblings[2])
        semantic_handler(cur -> siblings[2]);
    return NULL;
}

make_handler(compst) {
    new_scope();
    compst_without_scope(cur);
    free_scope();
    return NULL;
}

make_handler(logic) { //cur : Exp -> Exp AND Exp
    Type lhs = cur -> siblings[0] -> func(cur -> siblings[0]);
    Type rhs = cur -> siblings[2] -> func(cur -> siblings[2]);
    if(type_check(lhs, type_int, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH))
        return type_check(rhs, type_int, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH);
    return NULL;
}

make_handler(relop) {
    Type lhs = cur -> siblings[0] -> func(cur -> siblings[0]);
    Type rhs = cur -> siblings[2] -> func(cur -> siblings[2]);
    return type_check(lhs, rhs, type_int, cur -> siblings[0] -> lineno, OPERAND_MISMATCH);
}

make_handler(parentheses) { // cur : Exp -> LP Exp RP
    node* exp = cur -> siblings[1];
    return exp -> func(exp);
}

make_handler(uminus) {
    node* exp = cur -> siblings[1];
    return exp -> func(exp);
}

make_handler(not) { // cur : Exp -> NOT Exp
    node* exp = cur -> siblings[1];
    return exp -> func(exp);
}

make_handler(if) { // cur : Stmt -> IF LP Exp RP Stmt
                    //cur : Stmt -> IF LP Exp RP Stmt ELSE Stmt
    node* exp = cur -> siblings[2];
    type_check(exp -> func(exp), type_int, NULL, exp -> lineno, OPERAND_MISMATCH);
    semantic_handler(cur -> siblings[4]);
    if(cur -> cnt == 7)
        semantic_handler(cur -> siblings[6]);
    return NULL;
}

make_handler(while) { // cur : Stmt -> WHILE LP Exp RP Stmt
    node* exp = cur -> siblings[2];
    type_check(exp -> func(exp), type_int, NULL, exp -> lineno, OPERAND_MISMATCH);
    semantic_handler(cur -> siblings[4]);
    return NULL;
}
