#include "common.h"
#include "error.h"
#include "handlers.h"
#include "table.h"

char* get_vardec_name(node* cur) {
    do {
        cur = cur -> siblings[0];
    } while(!strcmp(cur -> name, "VarDec"));
    return cur -> val_str;
}

Type cur_def_type = NULL;
make_handler(def) {
    Assert(cur_def_type == NULL);
    cur_def_type = cur -> siblings[0] -> func(cur -> siblings[0]);
    if(cur -> cnt == 3) {
        preorder(cur -> siblings[1]);
    }
    cur_def_type = NULL;
}

make_handler(vardec) {
    node* vardec = cur -> siblings[0];
    if(table_insert(get_vardec_name(vardec), vardec -> func(vardec))) {
        semantic_error(cur -> lineno, REDEFINE_VARIABLE, get_vardec_name(cur -> siblings[0]));
    };
}

make_handler(variable) {
    return cur_def_type;
}
/*
make_handler(deflist) {
        cur = cur -> siblings[1];
        while(cur -> cnt == 3) {
            if(table_insert(get_vardec_name(cur -> siblings[0]), t)) {
                semantic_error(cur -> lineno, REDEFINE_VARIABLE, get_vardec_name(cur -> siblings[0]));
            };
            cur = cur -> siblings[2];
        }
        if(table_insert(get_vardec_name(cur -> siblings[0]), t)) {
            semantic_error(cur -> lineno, REDEFINE_VARIABLE, get_vardec_name(cur -> siblings[0]));
        }
}
*/

make_handler(type) {
    switch(cur -> siblings[0] -> val_int) {
        case T_FLOAT:
            return type_float;
        case T_INT:
            return type_int;
        default:
            panic();
    }
}

static inline FieldList deflist_to_structure(node* cur) {
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
            last -> type = t;
            while(vardec -> cnt == 3) {
                last -> type = to_array(last -> type, vardec -> siblings[2] -> val_int);
                vardec = vardec -> siblings[0];
            }
            last -> name = vardec -> siblings[0] -> val_str;
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
    return guard.next;
}

make_handler(struct_specifier) {
    cur = cur -> siblings[0];//Specifier to StructSpecifier
    char buf[100];
    switch(cur -> cnt) {
        case 2:
            sprintf(buf, "struct %s", cur -> siblings[1] -> siblings[0] -> val_str);
            Type ret = table_lookup(buf);
            if(!ret) {
                semantic_error(cur -> lineno, UNDEFINE_STRUCTURE, cur -> siblings[1] -> siblings[0] -> val_str);
            }
            return ret;
        case 5:
            {
                node* def_list = cur -> siblings[3];
                Type ret = new(struct Type_);
                ret -> kind = STRUCTURE;
                ret -> structure = deflist_to_structure(def_list);
                if(cur -> siblings[1]) {
                    sprintf(buf, "struct %s", cur -> siblings[1] -> siblings[0] -> val_str);
                    if(table_insert(buf, ret)) {
                        semantic_error(cur -> lineno, DUPLICATE_NAME, cur -> siblings[1] -> siblings[0] -> val_str);
                    }
                }
                return ret;
            }
        default:
            panic();
    }
}

make_handler(def_list) {
    panic();
    return NULL;
}

static Type cur_fun_ret_type = NULL;

make_handler(fun_dec) { // cur : Specifier FunDec CompSt
    //Type to_func(Type ret_val, int cnt, ...) {
    Type t = new(struct Type_);
    t -> kind = FUNCTION;
    t -> structure = new(struct FieldList_);
    t -> structure -> name = "";
    t -> structure -> type = cur -> siblings[0] -> func(cur -> siblings[0]);

    FieldList last = t -> structure;
    if(cur -> siblings[1] -> cnt == 4) {
        for(node* varlist = cur -> siblings[1] -> siblings[2]; ;varlist = varlist -> siblings[2]) {
            node* paramdec = varlist -> siblings[0];
            last -> next = new(struct FieldList_);
            last = last -> next;
            last -> name = get_vardec_name(paramdec -> siblings[1]);
            last -> type = paramdec -> siblings[0] -> func(paramdec -> siblings[0]);
            table_insert(last -> name, last -> type);
            if(varlist -> cnt == 1) {
                break;
            }
        }
    }
    last -> next = NULL;
    if(table_insert(cur -> siblings[1] -> siblings[0] -> val_str, t)) {
        semantic_error(cur -> siblings[1] -> siblings[0] -> lineno, REDEFINE_FUNCTION, cur -> siblings[1] -> siblings[0] -> val_str);
    }
    cur_fun_ret_type = t -> structure -> type;
    preorder(cur -> siblings[2]);
    cur_fun_ret_type = NULL;
    return NULL;
}

make_handler(null) {
    return NULL;
}

Type type_check(Type lhs, Type rhs, Type ret, int lineno, semantic_errors err, ...) {
    va_list ap;
    va_start(ap, err);

    if(lhs && rhs) {
        if(lhs -> kind == BASIC && rhs -> kind == BASIC) {
            if(lhs -> basic == rhs -> basic) {
                return lhs;
            }
        }
        vsemantic_error(lineno, err, ap);
    }
    if(ret) return ret;
    else return lhs;
}

int is_lvalue(node* cur) {
    switch(cur -> cnt) {
        case 1:
            return cur -> func == id_handler;
        case 3:
            //Exp DOT ID
            panic();
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
    int type_to_str(Type t, char* buf);
    if(cur -> cnt == 4) {
        FieldList param = func -> structure -> next;
        node* args = cur -> siblings[2];
        for(;
                args && param;
                args = args -> siblings[2], param = param -> next) {
            node* exp = args -> siblings[0];
            Type t = exp -> func(exp);
            if(t != param -> type) {
                break;
            }
            if(args -> cnt == 1) {
                args = NULL;
                break;
            }
        }
        if(param || args) {
            char buf1[100], buf2[100];
            type_to_str(func, buf1);
            int idx = 0;
            idx += sprintf(buf2 + idx, "(");
            for(args = cur -> siblings[2]; ;args = args -> siblings[2]) {
                node* exp = args -> siblings[0];
                Type t = exp -> func(exp);
                if(t -> basic == T_INT) {
                    idx += sprintf(buf2 + idx, "int");
                } else {
                    idx += sprintf(buf2 + idx, "float");
                }
                if(args -> cnt == 1) break;
                else idx += sprintf(buf2 + idx, ", ");
            }
            idx += sprintf(buf2 + idx, ")");
            semantic_error(fun -> lineno, FUNCTION_MISMATCH, fun -> val_str, buf1, buf2);
        }
    } else {
        if(func -> structure -> next) {
            char buf[100];
            type_to_str(func, buf);
            semantic_error(fun -> lineno, FUNCTION_MISMATCH, fun -> val_str, buf, "(void)");
        }
    }
    return NULL;
}

make_handler(array_access) {// cur : Exp LB Exp RB
    node* exp1 = cur -> siblings[0];
    Type base = exp1 -> func(exp1);
    if(base -> kind != ARRAY) {
        char buf[100];
        if(exp1 -> cnt == 1) {
            sprintf(buf, "%s", exp1 -> siblings[0] -> val_str);
        } else {
            TODO();
        }
        semantic_error(exp1 -> lineno, NOT_ARRAY, buf);
        return NULL;
    }
    node* exp2 = cur -> siblings[0];
    Type index = exp2 -> func(exp2);
    return type_check(type_int, index, base -> array.elem, exp2 -> lineno, NOT_INT, "1.5");
}

make_handler(return) {// cur : RETURN Exp SEMI
    Assert(cur_fun_ret_type);
    node* exp = cur -> siblings[1];
    type_check(cur_fun_ret_type, exp -> func(exp), NULL, exp -> lineno, RETURN_MISMATCH);
    return NULL;
}

make_handler(array_dec) {// cur : VarDec LB INT RB
    node* vardec = cur -> siblings[0];
    get_vardec_name(vardec);
    Type t = vardec -> func(vardec);
    return to_array(t, cur -> siblings[2] -> val_int);
}

make_handler(struct_access) {// cur : Exp DOT ID
    node* exp = cur -> siblings[0];
    Type t = exp -> func(exp);
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
