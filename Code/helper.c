#include "common.h"
#include "handlers.h"
#include "table.h"

const char* get_vardec_name(node* cur) {
    do {
        cur = cur -> siblings[0];
    } while(!strcmp(cur -> name, "VarDec"));
    return cur -> val_str;
}

make_handler(def) {
    Type t = cur -> siblings[0] -> func(cur -> siblings[0]);
    if(cur -> cnt == 3) {
        cur = cur -> siblings[1];
        while(cur -> cnt == 3) {
            table_insert(get_vardec_name(cur -> siblings[0]), t);
            cur = cur -> siblings[2];
        }
        table_insert(get_vardec_name(cur -> siblings[0]), t);
    }
}

make_handler(type) {
    switch(cur -> siblings[0] -> val_int) {
        case T_FLOAT:
            return type_float;
        case T_INT:
            return type_int;
        default:
            panic();
    }
}

static inline FieldList deflist_to_structure(node* cur) {
    struct FieldList_ guard;
    FieldList last = &guard;
    for(;cur && cur -> cnt == 2; cur = cur -> siblings[1]) {
        node* def = cur -> siblings[0];
        node* specifier = def -> siblings[0];
        Type t = specifier -> func(specifier);
        for(node* declist = def -> siblings[1];;) {
            node* vardec = declist -> siblings[0] -> siblings[0];
            last -> next = new(struct FieldList_);
            last = last -> next;
            last -> type = t;
            while(vardec -> cnt == 3) {
                last -> type = to_array(last -> type, vardec -> siblings[2] -> val_int);
                vardec = vardec -> siblings[0];
            }
            last -> name = vardec -> siblings[0] -> val_str;
            if(declist -> cnt == 1) {
                break;
            } else {
                declist = declist -> siblings[2];
            }
        }
    }
    last -> next = NULL;
    return guard.next;
}

make_handler(struct_specifier) {
    cur = cur -> siblings[0];//Specifier to StructSpecifier
    char buf[100];
    switch(cur -> cnt) {
        case 2:
            sprintf(buf, "struct %s", cur -> siblings[1] -> siblings[0] -> val_str);
            return table_lookup(buf);
        case 5:
            {
                node* def_list = cur -> siblings[3];
                Type ret = new(struct Type_);
                ret -> kind = STRUCTURE;
                ret -> structure = deflist_to_structure(def_list);
                if(cur -> siblings[1]) {
                    sprintf(buf, "struct %s", cur -> siblings[1] -> siblings[0] -> val_str);
                    table_insert(buf, ret);
                }
                return ret;
            }
        default:
            panic();
    }
}

make_handler(def_list) {
    panic();
    return NULL;
}

make_handler(null) {
    return NULL;
}
