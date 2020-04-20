#include "common.h"
#include "ir.h"

ir guard = {
    .prev = &guard,
    .next = &guard,
};

void add_ir(ir* i) {
    i->prev = guard.prev;
    i->next = &guard;
    i->prev->next = i->next->prev = i;
}

static void print_operand(operand op) {
    switch(op -> kind) {
        case VARIABLE:
            printf("%s", op -> val_str);
            break;
        case TEMP:
            printf("t%d", op -> t_no);
            break;
        case CONSTANT:
            printf("%d", op -> val_int);
            break;
        default:
            panic();
    }
}

void print_ir() {
    for(ir* i = guard.next; i != &guard; i = i -> next) {
        //printf("%p\n", i);
        if(i -> res) {
            print_operand(i -> res);
        } else {
            printf("Unknown");
        }
        printf(" = ");
        print_operand(i -> op1);
        printf(" + ");
        print_operand(i -> op2);
        putchar('\n');
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
