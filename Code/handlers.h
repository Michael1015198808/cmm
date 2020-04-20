#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"

#define make_handler(name) \
    void* name##_handler(node* cur, operand res)

void* semantic_handler(node* cur);

make_handler(def);
make_handler(variable);
make_handler(array_dec);
make_handler(extdeclist);
make_handler(vardec);
make_handler(type);
make_handler(struct_specifier);
make_handler(fun_dec);
make_handler(fun_def);
make_handler(return);
make_handler(assign);
make_handler(id);
make_handler(binary_op);
make_handler(int);
make_handler(float);
make_handler(fun_call);
make_handler(array_access);
make_handler(struct_access);
make_handler(compst);
make_handler(logic);
make_handler(relop);
make_handler(parentheses);
make_handler(uminus);
make_handler(not);
make_handler(if);
make_handler(while);

#endif
