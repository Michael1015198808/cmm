#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"

void* int_printer(node*);
void* float_printer(node*);
void* type_printer(node*);
void* id_printer(node*);

const char* get_vardec_name(node*);

#define make_handler(name) \
    void* name##_handler(node* cur)

make_handler(def);
make_handler(def_list);

make_handler(type);
make_handler(struct_specifier);

make_handler(null);

#endif
