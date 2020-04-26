#include "common.h"
#include "ir.h"

static ir guard = {
    .prev = &guard,
    .next = &guard,
    .func = NULL,
};

#define make_printer(name) \
    void name##_printer(ir* i)

make_printer(label);
make_printer(goto);

extern FILE* const out_file;

void remove_ir(ir* i) {
    i -> prev -> next = i -> next;
    i -> next -> prev = i -> prev;
}

void add_ir(ir* i) {
    i->prev = guard.prev;
    i->next = &guard;
    i->prev->next = i->next->prev = i;
}
static inline int output(const char* const fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if(out_file) {
        vfprintf(out_file, fmt, va);
    }
    va_start(va, fmt);
    return vprintf(fmt, va);
}

static void print_operand(operand op) {
    if(op) {
        switch(op -> kind) {
            case ADDRESS:
                output("*");
                return print_operand(op -> op);
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
    ret -> cnt = 0;
    ret -> no = ++no;
    return ret;
}

void print_label_goto(label l) {
    ir* cur_ir = new(ir);
    cur_ir -> func = goto_printer;
    cur_ir -> l = l;
    ++(l -> cnt);
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
void add_assign_ir(operand to, operand from) {
    ir* i = new(ir);
    i -> func = assign_printer;
    i -> res = to;
    i -> op1 = from;
    add_ir(i);
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
        operand tmp = new_temp_operand();
        i -> res = tmp;
        arith_printer(i);
        output("\n");
        print_operand(res);
        output(" := ");
        print_operand(tmp);
    }
}
void add_arith_ir(operand to, operand lhs, int arith_op, operand rhs) {
    ir* i = new(ir);
    i -> func = arith_printer;
    i -> val_int = arith_op;
    i -> op1 = lhs;
    i -> op2 = rhs;
    i -> res = to;
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
        operand tmp = new_temp_operand();
        i -> res = tmp;
        fun_call_printer(i);
        output("\n");
        print_operand(res);
        output(" := ");
        print_operand(tmp);
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
    output("PARAM %s", i -> val_str);
}
void add_param_ir(const char* name) {
    ir* i = new(ir);
    i -> func = param_printer;
    i -> val_str = name;
    add_ir(i);
}

make_printer(label) {
    output("LABEL l%d :", i -> l -> no);
}

make_printer(goto) { //called by if_nz, if_goto
    output("GOTO l%d", i -> l -> no);
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
    output("DEC r_%s %d\n", i -> val_str, i -> val_int);
    output("%s := &r_%s", i -> val_str, i -> val_str);
}
void add_dec_ir(const char* name ,unsigned size) {
    ir* i = new(ir);
    i -> func = dec_printer;
    i -> val_str = name;
    i -> val_int = size;
    add_ir(i);
}

static int dummy_goto() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == goto_printer) {
            if(cur -> next -> func == label_printer) {
                if(cur -> l == cur -> next -> l) {
                    ret = 1;
                    --cur -> l -> cnt;
                    remove_ir(cur);
                }
            }
        }
    }
    return ret;
}


static int dummy_label() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == label_printer) {
            Assert(cur -> l -> cnt <= 0x3f3f3f3f);
            if(cur -> l -> cnt == 0) {
                ret = 1;
                remove_ir(cur);
            }
        }
    }
    return ret;
}

/*
static int dummy_read() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> func == read_printer) {
            if(cur -> next -> func == assign_printer) {
                if(cur -> res == cur -> next -> op1) {
                    for(ir* other = cur -> next -> next; other != cur; other = other -> next) {
                        Assert(cur -> res != other -> op1);
                        Assert(cur -> res != other -> op2);
                    }
                    remove_ir(cur);
                    cur -> next -> func = read_printer;
                }
            }
        }
    }
    return ret;
}
*/
__attribute__((unused)) static int dummy_assign() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
        if(cur -> next -> func == assign_printer) {
            if(cur -> res == cur -> next -> op1) {
                for(ir* other = cur -> next -> next; other != cur; other = other -> next) {
                    Assert(cur -> res != other -> op1);
                    Assert(cur -> res != other -> op2);
                }
                ret = 1;
                cur -> res = cur -> next -> res;
                remove_ir(cur -> next);
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

static int template() {
    int ret = 0;
    for(ir* cur = guard.next; cur != &guard; cur = cur -> next) {
    }
    return ret;
}

static int(*opt_funcs[])() = {
    dummy_goto,
    dummy_label,
    //dummy_read,
    dummy_assign,
    dummy_temp,
    NULL,
};

void tot_optimize() {
    (void)template;
    int flag;
    do {
        flag = 0;
        for(int i = 0; opt_funcs[i]; ++i) {
            flag |= opt_funcs[i]();
        }
    } while(flag);
}
