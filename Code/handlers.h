#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"
#include "ir.h"

#define make_handler(name) \
    void* name##_handler(node* cur, operand res)

void* semantic(node* cur);

make_handler(stmt_exp);
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
make_handler(arith);
make_handler(int);
make_handler(float);
make_handler(fun_call);
make_handler(array_access);
make_handler(struct_access);
make_handler(compst);
make_handler(and);
make_handler(or);
make_handler(relop);
make_handler(parentheses);
make_handler(uminus);
make_handler(not);
make_handler(if);
make_handler(while);
make_handler(bool_to_int);

#define make_cond_handler(name) \
    void* name##_cond_handler(node* cur, label l1, label l2)

make_cond_handler(int_to_bool);
make_cond_handler(and);
make_cond_handler(or);
make_cond_handler(relop);
make_cond_handler(not);

#endif
