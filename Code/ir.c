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
            case VARIABLE:
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

operand new_temp_operand() {
    operand ret = new(struct operand_);
    return set_new_temp_operand(ret);
}

operand set_new_temp_operand(operand op) {
    static unsigned no = 0;
    op -> kind = TEMP;
    op -> t_no = ++no;
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
    print_operand(i -> res);
    output(" := ");
    print_operand(i -> op1);
    output(" %c ", i -> val_int);
    print_operand(i -> op2);
}

make_printer(function) {
    output("FUNCTION %s :", i -> val_str);
}

make_printer(write) {
    output("WRITE ");
    print_operand(i -> op1);
}

make_printer(fun_call) {
    print_operand(i -> res);
    output(" := CALL ");
    print_operand(i -> op1);
}

make_printer(fun_dec) {
    output("FUNCTION %s :", i -> val_str);
}

make_printer(param) {
    output("PARAM ");
    print_operand(i -> op1);
}
