#include "common.h"
#include "table.h"
#include "type.h"
static int offset = 0;

void add_variable(const char* s, unsigned size) {
    Type t = new(struct Type_);
    if (!table_lookup(s)) {
        t->kind = OFFSET;
        t->offset = (offset += size);
        table_insert(s, t);
    }
}

void add_int_variable(const char* s) {
    add_variable(s, 4);
}

void new_function() {
    free_scope();
    offset = 0;
    new_scope();
}
