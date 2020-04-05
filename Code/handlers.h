#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"

void* int_printer(node*);
void* float_printer(node*);
void* type_printer(node*);
void* id_printer(node*);

char* get_vardec_name(node*);

#define make_handler(name) \
    void* name##_handler(node* cur)

make_handler(assign);
make_handler(def);
make_handler(def_list);
make_handler(fun_dec);
make_handler(type);
make_handler(struct_specifier);
make_handler(null);
make_handler(id);
make_handler(binary_op);
make_handler(int);
make_handler(float);
make_handler(fun_call);
make_handler(array_access);
make_handler(array_dec);
make_handler(return);
make_handler(variable);
make_handler(vardec);
make_handler(struct_access);


#endif
