#include "common.h"
#include <stdlib.h>
#include "table.h"
#include "type.h"
#define CONTEXT_INFO 4
static int offset = CONTEXT_INFO;

void add_variable(operand op, unsigned size) {
    Type t = new(struct Type_);
    const char* const s = op_to_str(op);
    if (table_lookup(s)) {
        free((void*)s); // already exists, free the memory
    } else {
        if(size == 4) {
            t->kind = OFFSET_BASIC;
        } else {
            t->kind = OFFSET_COMP;
        }
        t->offset = (offset += size);
        table_insert(s, t);
    }
}

void add_int_variable(operand op) {
    add_variable(op, 4);
}

void new_function() {
    free_scope();
    offset = CONTEXT_INFO;
    new_scope();
}

int get_offset() {
    return offset;
}