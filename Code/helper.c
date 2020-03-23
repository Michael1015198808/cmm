#include "common.h"
#include "table.h"

const char* get_vardec_name(node* cur) {
    do {
        cur = cur -> siblings[0];
    } while(!strcmp(cur -> name, "VarDec"));
    return cur -> val_str;
}

void def_handler(node* cur) {
    Type t = get_type(cur -> siblings[0]);
    cur = cur -> siblings[1];
    while(cur -> cnt == 3) {
        table_insert(get_vardec_name(cur -> siblings[0]), t);
        cur = cur -> siblings[2];
    }
    table_insert(get_vardec_name(cur -> siblings[0]), t);
}
