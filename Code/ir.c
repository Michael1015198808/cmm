#include "optimization.h"
#include "common.h"
#include "ir.h"
void* memcpy(void*, const void*, size_t);

static ir guard = {
    .prev = &guard,
    .next = &guard,
    .func = NULL,
};

#define make_printer(name) \
    void name##_printer(ir* i)

make_printer(label);
make_printer(goto);
make_printer(if_goto);

extern FILE* const out_file;
int opcmp(operand op1, operand op2) {
    if(op1 && op2) {
        if(op1->kind == op2->kind) {
            switch(op1->kind) {
                case CONSTANT:
                    return op1->val_int!=op2->val_int;
                case POINTER:
                case VARIABLE:
                    return strcmp(op1->val_str, op2->val_str);
                case TEMP:
                    return op1->t_no!=op2->t_no;
                case ADDRESS:
                    return opcmp(op1->op, op2->op);
                default:
                    Assert(0);
                    return 1;
            }
        }
        return 1;
    } else {
        return op1 || op2;
    }
}

static inline label find(label* lp) {
    label l = *lp;
    if(l -> parent != l) {
        Assert(l -> cnt == 0);
        return *lp = l -> parent = find(&l -> parent);
    } else {
        return l;
    }
}

void remove_ir(ir* i) {
    i -> prev -> next = i -> next;
    i -> next -> prev = i -> prev;
    if(i -> func == goto_printer || i -> func == if_goto_printer) {
        --(find(&i -> l) -> cnt);
    }
}

static inline void add_ir_to_list(ir* i, ir* list_guard) {
    i->prev = list_guard -> prev;
    i->next = list_guard;
    i->prev->next = i->next->prev = i;
}
static void* add_ir(ir* i) {
    add_ir_to_list(i, &guard);
    return i;
}
#ifdef LOCAL
static inline int output(const char* const fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if(out_file) {
        return vfprintf(out_file, fmt, va);
    }
    va_start(va, fmt);
    return vprintf(fmt, va);
}
#else
static inline int output(const char* const fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if(out_file) {
        return vfprintf(out_file, fmt, va);
    } else {
        return vprintf(fmt, va);
    }
}
#endif

void test(ir* i) {
    i -> func(i);
    output("\n");
}
static void print_operand(operand op) {
    if(op) {
        switch(op -> kind) {
            case ADDRESS:
                output("*");
                print_operand(op -> op);
                return;
            case POINTER:
                Assert(op -> kind == POINTER);
                output("&");
            case VARIABLE:
#ifndef LOCAL
                output("v_");
#endif
                output("%s", op -> val_str);
                break;
            case TEMP:
                output("t%d", op -> t_no);
                break;
            case CONSTANT:
                output("#%d", op -> val_int);
                break;
            default:
                panic();
        }
    } else {
        output("Unknown");
    }
}

void print_ir() {
    for(ir* i = guard.next; i != &guard; i = i -> next) {
        //printf("%p\n", i);
        if(i -> func) {
            i -> func(i);
            output("\n");
        } else {
            panic();
        }
    }
}

label new_label() {
    static unsigned no = 0;
    label ret = new(struct label_);
    ret -> parent = ret;
    ret -> cnt = 0;
    ret -> no = ++no;
    return ret;
}

static inline void merge_label(label* lp1, label* lp2) {
    label l1 = find(lp1);
    label l2 = find(lp2);
    if(l1 != l2) {
        Assert(l1 -> no != l2 -> no);
        l1 -> parent = l2;
        l2 -> cnt += l1 -> cnt;
        l1 -> cnt = 0;
    }
}

void print_label_goto(label l) {
    ir* cur_ir = new(ir);
    cur_ir -> func = goto_printer;
    cur_ir -> l = l;
    ++l -> cnt;
    add_ir(cur_ir);
}

void print_label(label l) {
    ir* cur_ir = new(ir);
    cur_ir -> func = label_printer;
    cur_ir -> l = l;
    add_ir(cur_ir);
}

operand new_temp_operand() {
    operand ret = new(struct operand_);
    return set_temp_operand(ret);
}
operand set_temp_operand(operand op) {
    static unsigned no = 0;
    op -> kind = TEMP;
    op -> t_no = ++no;
    return op;
}

operand new_const_operand(int num) {
    operand ret = new(struct operand_);
    return set_const_operand(ret, num);
}
operand set_const_operand(operand op, int num) {
    op -> kind = CONSTANT;
    op -> val_int = num;
    return op;
}

operand new_variable_operand(const char* const name) {
    operand ret = new(struct operand_);
    return set_variable_operand(ret, name);
}

operand set_variable_operand(operand op, const char* const name) {
    op -> kind = VARIABLE;
    op -> val_str = name;
    return op;
}

operand new_dummy_operand() {
    operand ret = new(struct operand_);
    return set_dummy_operand(ret);
}
operand set_dummy_operand(operand op) {
    static unsigned no = 0;
    op -> kind = DUMMY;
    op -> t_no = ++no;
    return op;
}

make_printer(return) {
    output("RETURN ");
    print_operand(i -> op1);
}
void add_return_ir(operand op) {
    ir* i = new(ir);
    i -> func = return_printer;
    i -> op1 = op;
    add_ir(i);
}

make_printer(assign) {
    print_operand(i -> res);
    output(" := ");
    print_operand(i -> op1);
}
void* add_assign_ir(operand to, operand from) {
    ir* i = new(ir);
    i -> func = assign_printer;
    i -> res = to;
    i -> op1 = from;
    return add_ir(i);
}

make_printer(arith) {
    if(i -> res -> kind != ADDRESS) {
        print_operand(i -> res);
        output(" := ");
        print_operand(i -> op1);
        output(" %c ", i -> val_int);
        print_operand(i -> op2);
    } else {
        operand res = i -> res;
        i -> res = new_temp_operand();
        arith_printer(i);
        output("\n");
        print_operand(res);
        output(" := ");
        print_operand(i -> res);
        free(i -> res);
        i -> res = res;
    }
}
void add_arith_ir(operand res, operand op1, int arith_op, operand op2) {
    if(OPTIMIZE(ARITH_CONSTANT)) {
        if(op1 -> kind == CONSTANT && op2 -> kind == CONSTANT) {
            res -> kind = CONSTANT;
            switch(arith_op) {
                case '+':
                    res -> val_int = op1 -> val_int + op2 -> val_int;
                    break;
                case '-':
                    res -> val_int = op1 -> val_int - op2 -> val_int;
                    break;
                case '*':
                    res -> val_int = op1 -> val_int * op2 -> val_int;
                    break;
                case '/':
                    res -> val_int = op1 -> val_int / op2 -> val_int;
                    break;
            }
            free(op1);
            free(op2);
            return;
        } else {
            operand cons = NULL, other = NULL;
            if(op1 -> kind == CONSTANT) {
                cons = op1;
            } else {
                other = op1;
            }
            if(op2 -> kind == CONSTANT) {
                cons = op2;
            } else {
                other = op2;
            }
            if(cons) {
                switch(arith_op) {
                    case '-':
                        if(cons == op1)break;
                        //other - cons
                    case '+':
                        //other + cons or cons + other
                        if(cons -> val_int == 0) {
                            memcpy(res, other, sizeof(*res));
                            return;
                        }
                        break;
                    case '/':
                        if(cons == op1) {
                            //cons / other
                            if(cons -> val_int == 0) {
                                set_const_operand(res, 0);
                                return;
                            }
                            break;
                        }
                        //other / cons
                    case '*':
                        //other * cons or cons * other
                        if(cons -> val_int == 1) {
                            memcpy(res, other, sizeof(*res));
                            return;
                        } else if(cons -> val_int == 0) {
                            set_const_operand(res, 0);
                            free(op1);
                            free(op2);
                            return;
                        }
                        break;
                }
            } else if(!opcmp(op1, op2) && arith_op == '/') {
                set_const_operand(res, 1);
                return;
            }
        }
    }
    ir* i = new(ir);
    i -> func = arith_printer;
    i -> val_int = arith_op;
    i -> op1 = op1;
    i -> op2 = op2;
    i -> res = res;
    add_ir(i);
}

make_printer(write) {
    output("WRITE ");
    print_operand(i -> op1);
}
void add_write_ir(operand op) {
    ir* i = new(ir);
    i -> func = write_printer;
    i -> op1 = op;
    add_ir(i) ;
}

make_printer(read) {
    output("READ ");
    print_operand(i -> res);
}
void add_read_ir(operand op) {
    ir* i = new(ir);
    i -> func = read_printer;
    i -> res = op;
    add_ir(i);
}

make_printer(fun_call) {
    if(i -> res -> kind != ADDRESS) {
        print_operand(i -> res);
        output(" := CALL %s", i -> val_str);
    } else {
        operand res = i -> res;
        i -> res = new_temp_operand();
        fun_call_printer(i);
        output("\n");
        print_operand(res);
        output(" := ");
        print_operand(i -> res);
        free(i -> res);
        i -> res = res;
    }
}
void add_fun_call_ir(const char* name, operand op) {
    ir* i = new(ir);
    i -> func = fun_call_printer;
    i -> val_str = name;
    i -> res = op;
    add_ir(i);
}

make_printer(fun_dec) {
    output("FUNCTION %s :", i -> val_str);
}
void add_fun_dec_ir(const char* name) {
    ir* i = new(ir);
    i -> func = fun_dec_printer;
    i -> val_str = name;
    add_ir(i);
}

make_printer(param) {
    output("PARAM ");
    print_operand(i -> op1);
}

static ir params_guard = {
    .prev = &params_guard,
    .next = &params_guard,
    .func = NULL,
};

void add_param_ir_flush(const char* name) {
    for(ir *i = params_guard.prev; i != &params_guard;) {
        ir *prev = i -> prev;
        remove_ir(i);
        add_ir(i);
        i = prev;
    }
}

void add_param_ir_buffered(const char* name) {
    ir* i = new(ir);
    i -> func = param_printer;
    i -> op1 = new_variable_operand(name);
    add_ir_to_list(i, &params_guard);
}

make_printer(label) {
    output("LABEL l%d :", find(&i -> l) -> no);
}

make_printer(goto) { //called by if_nz, if_goto
    output("GOTO l%d", find(&i -> l) -> no);
}

make_printer(if_goto) {
    output("IF ");
    print_operand(i -> op1);
    output(" %s ", i -> val_str);
    print_operand(i -> op2);
    output(" ");
    goto_printer(i);
}
void add_if_goto_ir(operand op1, operand op2, const char* cmp, label l) {
    ir* i = new(ir);
    i -> func = if_goto_printer;
    i -> op1 = op1;
    i -> op2 = op2;
    i -> val_str = cmp;
    i -> l = l;
    ++(l -> cnt);
    add_ir(i);
}
void add_if_nz_ir(operand op1, label l) {
    add_if_goto_ir(op1, new_const_operand(0), "!=", l);
}

make_printer(arg) {
    output("ARG ");
    print_operand(i -> op1);
}
void add_arg_ir(operand op) {
    ir* i = new(ir);
    i -> func = arg_printer;
    i -> op1 = op;
    add_ir(i);
}

make_printer(dec) {
    output("DEC r_%s %d\n", i -> op1 -> val_str, i -> val_int);
    print_operand(i -> op1);
    output(" := &r_%s", i -> op1 -> val_str);
}
void add_dec_ir(const char* name ,unsigned size) {
    ir* i = new(ir);
    i -> func = dec_printer;
    i -> op1 = new_variable_operand(name);
    i -> val_int = size;
    add_ir(i);
}

//remove goto next line
//goto l2
//LABEL l2:
static int dummy_goto() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == goto_printer) {
            if(cur -> next -> func == label_printer) {
                if(find(&cur -> l) == find(&cur -> next -> l)) {
                    ret = 1;
                    remove_ir(cur);
                }
            }
        }
    }
    return ret;
}


static int dummy_label() { //remove labels that will not be jumped to
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == label_printer) {
            Assert(cur -> l -> cnt <= 0x3f3f3f3f);
            if(find(&cur -> l)->cnt == 0) {
                ret = 1;
                remove_ir(cur);
            }
        }
    }
    return ret;
}

__attribute__((unused)) static int chain_assign() { //remove dummy chain assign
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> next -> func == assign_printer) {
            if( cur -> res &&
              ( cur -> res -> kind == TEMP||
                cur -> res -> kind == CONSTANT)) {
                if(!opcmp(cur -> res, cur -> next -> op1)) {
                    ret = 1;
                    cur -> res = cur -> next -> res;
                    remove_ir(cur -> next);
                }
            }
        }
    }
    return ret;
}

//remove chain goto
//GOTO l2
//...
//LABEL l2:
//GOTO l3
static int chain_goto() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == label_printer) {
            if(cur -> next -> func == goto_printer) {
                ret = 1;
                merge_label(&cur -> l, &cur -> next -> l);
                remove_ir(cur);
            }
        }
    }
    return ret;
}

static int dummy_temp() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> res) {
            if(cur -> res -> kind == DUMMY) {
                ret = 1;
                remove_ir(cur);
            }
        }
    }
    return ret;
}

static int adj_label() { //remove adjacent label
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == label_printer) {
            if(cur -> next -> func == label_printer) {
                ret = 1;
                merge_label(&cur -> l, &cur -> next -> l);
                remove_ir(cur);
            }
        }
    }
    return ret;
}

//remove unreachable statements
//e.g., statements after RETURN, GOTO
static int remove_unreachable() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == return_printer || cur -> func == goto_printer) {
            for(ir* i = cur -> next; i != &guard; i = i -> next) {
                if(i -> func == label_printer) {
                    break;
                } else if(i -> func == fun_dec_printer) {
                    break;
                } else {
                    ret = 1;
                    remove_ir(i);
                }
            }
        }
    }
    return ret;
}

char* revert_relop(const char* relop) {
    switch(relop[0]) {
        case '!':
            return "==";
        case '=':
            return "!=";
        case '<':
            return relop[1] == '='?">":">=";
        case '>':
            return relop[1] == '='?"<":"<=";
        default:
            Assert(0);
    }
    return NULL;
}

static int redundant_goto() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur->func == if_goto_printer &&
                cur->next->func == goto_printer &&
                cur->next->next->func == label_printer) {
            if(find(&cur->l)->no == find(&cur->next->next->l)->no) {
                ret = 1;
                --cur->l->cnt;
                cur->val_str = revert_relop(cur->val_str);
                cur->l = find(&cur->next->l);
                ++cur->l->cnt;
                remove_ir(cur->next);
            }
        }
    }
    return ret;
}

static int template() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
    }
    return ret;
}

static struct {
    int (*func)(void);
    int level;
} optimizers[] = {
    {dummy_goto, 1},
    {redundant_goto, 1}, //dragon book 6.6.5
    {dummy_label, 1},
    {chain_assign, 1},
    {chain_goto, 1}, //dragon book 8.7.3
    {adj_label, 1},
    {dummy_temp, 1},
    {remove_unreachable, 1}, //dragon book 8.7.2
    {NULL, 0},
};

void tot_optimize() {
    (void)template;
    int flag;
    do {
        flag = 0;
        for(int i = 0; optimizers[i].func; ++i) {
            if(OPTIMIZE(optimizers[i].level))
                flag |= optimizers[i].func();
        }
    } while(flag);
}
