#include "common.h"
#include "type.h"
#include "error.h"
#include "handlers.h"
#include "table.h"
#include "optimization.h"

#define call_with_first_four(func, a, b, c, d, ...) \
    func(a, b, c, d)

#define semantic_handler(cur, ...) \
    call_with_first_four(cur -> func, cur, ##__VA_ARGS__, NULL, NULL, NULL)

/*
static inline void* semantic_handler(node* cur, operand op) {
    return cur -> func(cur, op, NULL, NULL);
}
*/

void* semantic(node* cur) { // Most top-level handler. Default handler
    if(cur -> func) {
        return cur -> func(cur, NULL, NULL, NULL);
    } else {
        for(int i = 0; i < cur->cnt; ++i) {
            if(cur->siblings[i])
                semantic(cur->siblings[i]);
        }
        return NULL;
    }
}
static inline void* condition(node* cur, label l1, label l2) {
    return cur -> cond(cur, l1, l2);
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
    cur_def_type = semantic(cur -> siblings[0]);
    if(cur -> cnt == 3) {
        semantic(cur -> siblings[1]);
    }
    cur_def_type = old_def_type;
    return NULL;
}

make_handler(variable) {// cur : VarDec -> ID
    if(res) {
        res -> kind = VARIABLE;
        res -> val_str = cur -> siblings[0] -> val_str;
    }
    return cur_def_type;
}

make_handler(array_dec) {// cur : VarDec -> VarDec LB INT RB
    node* vardec = cur -> siblings[0];
    Type t = semantic(vardec);
    return to_array(t, cur -> siblings[2] -> val_int);
}

make_handler(extdeclist) {
    node* vardec = cur -> siblings[0];
    const char* const name = get_vardec_name(vardec);
    if(table_insert(name, semantic(vardec))) {
        semantic_error(cur -> lineno, REDEFINE_VARIABLE, name);
    }
    return semantic(cur -> siblings[2]);
}

make_handler(vardec) {// cur : Dec -> VarDec
                      // cur : ExtDecList -> VarDec
                      // cur : Dec -> VarDec ASSIGNOP Exp
    node* vardec = cur -> siblings[0];
    const char* const name = get_vardec_name(vardec);
    operand op = new(struct operand_);
    Type t = semantic_handler(vardec, op);
    if(t) {
        if(table_insert(name, t)) {
            semantic_error(cur -> lineno, REDEFINE_VARIABLE, name);
        }
        if(t -> kind != BASIC) {
            add_dec_ir(name, t -> size);
        } else if(cur -> cnt == 3) {
            operand op1 = new(struct operand_);
            node* exp = cur -> siblings[2];
            type_check(t, semantic_handler(exp, op1), NULL, exp -> lineno, ASSIGN_MISMATCH);
            add_assign_ir(op, op1);
        }
    }
    return NULL;
}

make_handler(type) {
    switch(cur -> siblings[0] -> val_int) {
        case T_FLOAT:
            return (void*)type_float;
        case T_INT:
            return (void*)type_int;
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
        Type t = semantic(specifier);
        for(node* declist = def -> siblings[1];; declist = declist -> siblings[2]) {
            node* vardec = declist -> siblings[0] -> siblings[0];
            last -> next = new(struct FieldList_);
            last = last -> next;
            cur_def_type = t;
            last -> type = semantic(vardec);
            last -> name = get_vardec_name(vardec);
            ret -> size += last -> type -> size;
            if(declist -> siblings[0] -> cnt == 3) {
                semantic_error(vardec -> siblings[0] -> lineno, REDEFINE_FIELD, last -> name);
            }
            if(table_insert(last -> name, last -> type)) {
                semantic_error(vardec -> siblings[0] -> lineno, REDEFINE_FIELD, last -> name);
            }
            if(declist -> cnt == 1) {
                break;
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
            if((ret = table_lookup_struct(cur -> siblings[1] -> siblings[0] -> val_str))) {
                ret = ret -> variable;
            } else {
                semantic_error(cur -> lineno, UNDEFINE_STRUCTURE, cur -> siblings[1] -> siblings[0] -> val_str);
            }
            break;
        case 5:
            ret = new(struct Type_);
            ret -> kind = STRUCTURE_DEF;
            new_scope();
            ret -> variable = deflist_to_struct_type(cur -> siblings[3]);
            struct_free_scope();
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
    ret -> structure -> type = semantic(cur -> siblings[0]);

    FieldList last = ret -> structure;
    if(cur -> siblings[1] -> cnt == 4) {
        for(node* varlist = cur -> siblings[1] -> siblings[2]; ;varlist = varlist -> siblings[2]) {
            node* paramdec = varlist -> siblings[0];
            cur_def_type = semantic(paramdec -> siblings[0]);

            last -> next = new(struct FieldList_);
            last = last -> next;
            last -> name = get_vardec_name(paramdec -> siblings[1]);
            last -> type = semantic(paramdec -> siblings[1]);
            Type t = semantic(paramdec -> siblings[1]);
            if(table_insert(last -> name, t)) {
                semantic_error(paramdec -> lineno, REDEFINE_VARIABLE, last -> name);
            };
            add_param_ir(last -> name);
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
    const char* const fun_name = cur -> siblings[1] -> siblings[0] -> val_str;
    new_scope();
    add_fun_dec_ir(fun_name);
    Type t = fun_parsing(cur, 0);
    if(table_insert_global(fun_name, t)) {
        Type old = table_lookup(fun_name);
        if(old -> is_dec) {
            old -> is_dec = 0;
            if(typecmp(old, t)) {
                semantic_error(cur -> siblings[1] -> siblings[0] -> lineno, INCONSIS_DEC, fun_name);
            }
        } else {
            semantic_error(cur -> siblings[1] -> siblings[0] -> lineno, REDEFINE_FUNCTION, fun_name);
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
    Assert(res == NULL);
    node* exp = cur -> siblings[1];
    operand op1 = NULL;
    type_check(cur_fun_ret_type, semantic_handler(exp, op1 = new(struct operand_)), NULL,
            exp -> lineno, RETURN_MISMATCH);
    add_return_ir(op1);
    return NULL;
}

make_handler(struct_access) {// cur : Exp DOT ID
    node* exp = cur -> siblings[0];
    operand op1;
    Type t = semantic_handler(exp, op1 = new(struct operand_));
    if(t) {
        if(t -> kind != STRUCTURE) {
            semantic_error(exp -> lineno, ILLEGAL_USE, ".");
            return NULL;
        }
        unsigned offset = 0;
        for(FieldList f = t -> structure;f;f = f -> next) {
            if(!strcmp(f -> name, cur -> siblings[2] -> val_str)) {
                if(f -> type -> kind == BASIC) {
                    res -> kind = ADDRESS;
                    res = (res -> op = new_temp_operand());
                } else {
                    res = set_temp_operand(res);
                }
                add_arith_ir(res, op1, '+', new_const_operand(offset));

                return f -> type;
            }
            offset += f -> type -> size;
        }
        semantic_error(exp -> lineno, NONEXIST_FIELD, cur -> siblings[2] -> val_str);
    }
    return NULL;
}

make_handler(array_access) {// cur : Exp LB Exp RB
    node* exp1 = cur -> siblings[0];
    operand array_base_address, idx, base_size, offset = new_temp_operand();
    Type base = semantic_handler(exp1, array_base_address = new_temp_operand());
    if(base -> kind != ARRAY) {
        semantic_error(exp1 -> lineno, NOT_ARRAY);
        return NULL;
    }
    node* exp2 = cur -> siblings[2];
    Type index = semantic_handler(exp2, idx = new_temp_operand());
    if(res) {
        base_size = new_const_operand(base -> array.elem -> size);

        add_arith_ir(offset, idx, '*', base_size);

        if(base -> array.elem -> kind == BASIC) {
            res -> kind = ADDRESS;
            res = (res -> op = new_temp_operand());
        } else {
            res = set_temp_operand(res);
        }
        add_arith_ir(res, offset, '+', array_base_address);
    }

    return type_check(type_int, index, base -> array.elem, exp2 -> lineno, NOT_INT);
}

make_handler(assign) { //cur : Exp -> Exp ASSIGNOP Exp
    operand op2;
    if(!res) {
        res = new(struct operand_);
    }
    Type lhs = semantic_handler(cur -> siblings[0], res);
    IF(res -> kind != VARIABLE && res -> kind != ADDRESS) {
        semantic_error(cur -> siblings[0] -> lineno, LVALUE);
        return NULL;
    }
    Type rhs = semantic_handler(cur -> siblings[2], op2 = new(struct operand_), l1, l2);
    if(op2 -> kind == NONE) {
        label_add_true(l1, res);
        label_add_false(l2, res);
    } else {
        add_assign_ir(res, op2);
        if(l1 || l2) {
            add_if_nz_ir(res, l1);
            print_label_goto(l2);
        }
    }
    return type_check(lhs, rhs, NULL, cur -> siblings[0] -> lineno, ASSIGN_MISMATCH);
}

make_handler(id) { //cur : Exp -> ID
    node* id = cur -> siblings[0];
    Type t = table_lookup(id -> val_str);
    if(!t) {
        semantic_error(id -> lineno, UNDEFINE_VARIABLE, id -> val_str);
    }
    if(l1 || l2) {
        if(!res) {
            res = new_temp_operand();
        }
        add_if_nz_ir(res, l1);
        print_label_goto(l2);
    }
    if(res) {
        res -> kind = VARIABLE;
        res -> val_str = id -> val_str;
    }
    return t;
}

make_handler(arith) {
    operand op1, op2;
    Type lhs = semantic_handler(cur -> siblings[0], op1 = new(struct operand_));
    Type rhs = semantic_handler(cur -> siblings[2], op2 = new(struct operand_));
    operand tmp;
    if(res) {
        tmp = set_temp_operand(res);
    } else {
        tmp = new_temp_operand();
    }
    if(OPTIMIZE(ARITH_CONSTANT)
            && op1 -> kind == CONSTANT && op2 -> kind == CONSTANT) {
        tmp -> kind = CONSTANT;
        switch(cur -> val_int) {
            case '+':
                tmp -> val_int = op1 -> val_int + op2 -> val_int;
                break;
            case '-':
                tmp -> val_int = op1 -> val_int - op2 -> val_int;
                break;
            case '*':
                tmp -> val_int = op1 -> val_int * op2 -> val_int;
                break;
            case '/':
                tmp -> val_int = op1 -> val_int / op2 -> val_int;
                break;
        }
        free(op1);
        free(op2);
    } else {
        add_arith_ir(tmp, op1, cur -> val_int, op2);
    }
    if(!res) {
        add_if_nz_ir(tmp, l1);
        print_label_goto(l2);
    }
    return type_check(lhs, rhs, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH);
}

make_handler(bool_to_int) {
    label ctrue  = new_label();
    label cfalse = new_label();
    label cfinal = new_label();
    void* ret = condition(cur, ctrue, cfalse);
    print_label(ctrue);
    add_assign_ir(res, new_const_operand(1));
    print_label_goto(cfinal);
    print_label(cfalse);
    add_assign_ir(res, new_const_operand(0));
    print_label(cfinal);
    return ret;
}

make_handler(int) {
    if(res) {
        res -> kind = CONSTANT;
        res -> val_int = cur -> siblings[0] -> val_int;
    } else if(l1 || l2) {
        if(cur -> siblings[0] -> val_int) {
            print_label_goto(l1);
        } else {
            print_label_goto(l2);
        }
    }
    return (void*)type_int;
}

make_handler(float) {
    panic();
    return (void*)type_float;
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
    if(!strcmp(fun -> val_str, "write")) {
        operand op;
        semantic_handler(cur -> siblings[2] -> siblings[0], op = new(struct operand_));
        add_write_ir(op);
    } else if(!strcmp(fun -> val_str, "read")) {
        if(res) {
            add_read_ir(set_temp_operand(res));
        }
    } else {
        if(cur -> cnt == 4) {
            FieldList param = func -> structure -> next;
            node* args = cur -> siblings[2];
            for(; param;) {
                node* exp = args -> siblings[0];
                operand op;
                if(typecmp(semantic_handler(exp, op = new(struct operand_)), param -> type)) {
                    break;
                }
                add_arg_ir(op);
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
        if(res) {
            set_temp_operand(res);
        } else {
            res = new_temp_operand();
        }
        add_fun_call_ir(fun -> val_str, res);
    }
    return func -> structure -> type;
}



static void* compst_without_scope(node* cur) {
    if(cur -> siblings[1])
        semantic(cur -> siblings[1]);
    if(cur -> siblings[2])
        semantic(cur -> siblings[2]);
    return NULL;
}

make_handler(compst) {
    new_scope();
    compst_without_scope(cur);
    free_scope();
    return NULL;
}

make_handler(and) { //cur : Exp -> Exp AND Exp
    if(res) res -> kind = NONE;
    label e1true = new_label();
    Type lhs = semantic_handler(cur -> siblings[0], NULL, e1true, l2);
    print_label(e1true);
    Type rhs = semantic_handler(cur -> siblings[2], NULL, l1, l2);
    if(type_check(lhs, type_int, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH))
        return type_check(rhs, type_int, NULL, cur -> siblings[2] -> lineno, OPERAND_MISMATCH);
    return NULL;
}

make_handler(or) { //cur : Exp -> Exp AND Exp
    if(res) res -> kind = NONE;
    label e1false = new_label();
    Type lhs = semantic_handler(cur -> siblings[0], NULL, l1, e1false);
    print_label(e1false);
    Type rhs = semantic_handler(cur -> siblings[2], NULL, l1, l2);
    if(type_check(lhs, type_int, NULL, cur -> siblings[0] -> lineno, OPERAND_MISMATCH))
        return type_check(rhs, type_int, NULL, cur -> siblings[2] -> lineno, OPERAND_MISMATCH);
    return NULL;
}

static inline int const_relop_cal(int val1, int val2, const char* cmp) {
    switch(cmp[0]) {
        case '!':
            return val1 != val2;
        case '=':
            return val1 == val2;
        case '>':
            return val1 > val2 || const_relop_cal(val1, val2, cmp + 1);
        case '<':
            return val1 < val2 || const_relop_cal(val1, val2, cmp + 1);
        case '\0':
            return 0;
        default:
            panic();
            return 0;
    }
}
make_handler(relop) {// cur : Exp -> Exp RELOP Exp
    if(res) res -> kind = NONE;
    operand op1, op2;
    Type lhs = semantic_handler(cur -> siblings[0], op1 = new_temp_operand());
    Type rhs = semantic_handler(cur -> siblings[2], op2 = new_temp_operand());
    Assert((!((res) && (l1 || l2))));
    Assert(((res) || (l1 || l2)));
    if(OPTIMIZE(ARITH_CONSTANT)
            && op1 -> kind == CONSTANT && op2 -> kind == CONSTANT) {
        if(res) {
            res -> kind = CONSTANT;
            res -> val_int = 1;
        } else {
            const char* cmp = cur -> siblings[1] -> val_str;
            print_label_goto(const_relop_cal(op1 -> val_int, op2 -> val_int, cmp)? l1 : l2);
            free(op1);
            free(op2);
        }
    } else {
        if(!l2) {
            add_assign_ir(set_temp_operand(res), new_const_operand(1));
            l1 = new_label();
        }
        add_if_goto_ir(op1, op2, cur -> siblings[1] -> val_str, l1);
        if(!l2) {
            add_assign_ir(res, new_const_operand(0));
            print_label(l1);
        } else {
            print_label_goto(l2);
        }
    }
    return type_check(lhs, rhs, type_int, cur -> siblings[0] -> lineno, OPERAND_MISMATCH);
}

make_handler(parentheses) { // cur : Exp -> LP Exp RP
    return semantic_handler(cur -> siblings[1], res, l1, l2);
}

make_handler(uminus) {
    operand tmp;
    if(res) {
        tmp = set_temp_operand(res);
    } else {
        tmp = new_temp_operand();
    }
    operand op1;
    void* ret = semantic_handler(cur -> siblings[1], op1 = new_temp_operand());
    if(OPTIMIZE(ARITH_CONSTANT) && op1 -> kind == CONSTANT) {
        tmp -> kind = CONSTANT;
        tmp -> val_int = -op1 -> val_int;
        free(op1);
    } else {
        add_arith_ir(tmp, 0, '-', op1);
    }
    if(!res) {
        add_if_nz_ir(tmp, l1);
        print_label_goto(l2);
    }
    return ret;
}

make_handler(not) { // cur : Exp -> NOT Exp
    node* exp = cur -> siblings[1];
    if(l1 || l2) {
        Assert(res == NULL);
        return semantic_handler(exp, NULL, l2, l1);
    } else {
        Assert(res);
        label ctrue = new_label(), cfalse = new_label(), cfinal = new_label();
        Type ret = semantic_handler(exp, NULL, cfalse, ctrue);
        set_temp_operand(res);
        print_label(ctrue);
        add_assign_ir(res, new_const_operand(1));
        print_label_goto(cfinal);
        print_label(cfalse);
        add_assign_ir(res, new_const_operand(0));
        print_label(cfinal);
        return ret;
    }
}

make_handler(if) { // cur : Stmt -> IF LP Exp RP Stmt
                    //cur : Stmt -> IF LP Exp RP Stmt ELSE Stmt
    node* exp = cur -> siblings[2];
    label ctrue  = new_label(),
          cfalse = new_label();
    type_check(semantic_handler(exp, NULL, ctrue, cfalse), type_int, NULL, exp -> lineno, OPERAND_MISMATCH);
    if(ctrue -> cnt) {
        print_label(ctrue);
        semantic(cur -> siblings[4]);
    } else {
        free(ctrue);
    }
    label cfinal = NULL;
    if(cur -> cnt == 7 || cfalse -> tlist || cfalse -> flist) {
        cfinal = new_label();
        print_label_goto(cfinal);
    }
    if(cfalse -> cnt) {
        print_label(cfalse);
        if(cur -> cnt == 7)
            semantic(cur -> siblings[6]);
    } else {
        free(cfalse);
    }
    if(cfinal) {
        print_label(cfinal);
    }
    return NULL;
}

make_handler(while) { // cur : Stmt -> WHILE LP Exp RP Stmt
    label wtrue  = new_label(),
          wfalse = new_label(),
          start  = new_label();
    node* exp = cur -> siblings[2];
    print_label(start);
    type_check(semantic_handler(exp, NULL, wtrue, wfalse), type_int, NULL, exp -> lineno, OPERAND_MISMATCH);
    print_label(wtrue);
    semantic(cur -> siblings[4]);
    print_label_goto(start);
    print_label(wfalse);
    return NULL;
}

make_cond_handler(int_to_bool) {
    operand op;
    void* ret = semantic_handler(cur, op = new(struct operand_));
    add_if_nz_ir(op, l1);
    print_label_goto(l2);
    return ret;
}
