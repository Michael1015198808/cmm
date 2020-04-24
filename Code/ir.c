#include "common.h"
#include "ir.h"

//#define PREDEFINE_MAIN
#ifdef PREDEFINE_MAIN
static ir main_dec_ir;
static ir guard = {
    .prev = &main_dec_ir,
    .next = &main_dec_ir,
};
static ir main_dec_ir = {
    .prev = &guard,
    .next = &guard,
    .func = function_printer,
    .val_str = "main",
};
#else
static ir guard = {
    .prev = &guard,
    .next = &guard,
};
#endif

static struct operand_ op_zero_ = {
    .kind = CONSTANT,
    .val_int = 0,
};
const operand op_zero = &op_zero_;

extern FILE* const out_file;

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
    cur_ir -> val_int = l -> no;
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

make_printer(assign) {
    print_operand(i -> res);
    output(" := ");
    print_operand(i -> op1);
}

make_printer(binary) {
    if(i -> res -> kind != ADDRESS) {
        print_operand(i -> res);
        output(" := ");
        print_operand(i -> op1);
        output(" %c ", i -> val_int);
        print_operand(i -> op2);
    } else {
        operand tmp = new_temp_operand();
        print_operand(tmp);
        output(" := ");
        print_operand(i -> op1);
        output(" %c ", i -> val_int);
        print_operand(i -> op2);
        output("\n");
        print_operand(i -> res);
        output(" := ");
        print_operand(tmp);
    }
}

make_printer(function) {
    output("FUNCTION %s :", i -> val_str);
}

make_printer(write) {
    output("WRITE ");
    print_operand(i -> op1);
}

make_printer(read) {
    output("READ ");
    print_operand(i -> res);
}

make_printer(fun_call) {
    print_operand(i -> res);
    output(" := CALL ");
    print_operand(i -> op1);
}

make_printer(fun_dec) {
    output("FUNCTION %s :", i -> res -> val_str);
}

make_printer(param) {
    output("PARAM %s", i -> res -> val_str);
}

make_printer(label) {
    output("LABEL l%d :", i -> val_int);
}

make_printer(goto) {
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

make_printer(arg) {
    output("ARG ");
    print_operand(i -> op1);
}

make_printer(struct_dec) {
    output("DEC r_%s %d\n", i -> res -> val_str, i -> val_int);
    output("%s := &r_%s", i -> res -> val_str, i -> res -> val_str);
}

make_printer(dec) {
    output("DEC %s %d", i -> res -> val_str, i -> val_int);
}
