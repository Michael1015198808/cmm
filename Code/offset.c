#include "common.h"
#include <stdlib.h>
#include "table.h"
#include "type.h"
#define CONTEXT_INFO 0
#define INT_SIZE 4

static int offset = CONTEXT_INFO;

static void add_variable_real(operand op, int is_basic, unsigned size) {
    Type t = new(struct Type_);
    const char* const s = op_to_str(op);
    if (table_lookup(s)) {
        free((void*)s); // already exists, free the memory
    } else {
        if(is_basic) {
            t->kind = OFFSET_BASIC;
        } else {
            t->kind = OFFSET_COMP;
        }
        //int output(const char* const fmt, ...);
        //output("#%s from %d to %d\n", s, offset, offset-size);
        t->offset = (offset -= size);
        table_insert(s, t);
    }
}

void add_comp_variable(operand op, unsigned size) {
    add_variable_real(op, 0, size);
}
void add_int_variable(operand op) {
    add_variable_real(op, 1, INT_SIZE);
}

void new_function() {
    free_scope();
    offset = CONTEXT_INFO;
    new_scope();
}

int get_offset() {
    return offset;
}
