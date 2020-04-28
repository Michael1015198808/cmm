#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"
#include "ir.h"

#define make_semantic_handler(name) \
    void* name##_semantic_handler(node* cur)

#define make_arith_handler(name) \
    void* name##_arith_handler(node* cur, operand res)

#define make_cond_handler(name) \
    void* name##_cond_handler(node* cur, label l1, label l2)

void* semantic(node* cur);

make_semantic_handler(stmt_exp);
make_semantic_handler(def);
make_semantic_handler(variable);
make_semantic_handler(array_dec);
make_semantic_handler(extdeclist);
make_semantic_handler(vardec);
make_semantic_handler(type);
make_semantic_handler(struct_specifier);
make_semantic_handler(fun_dec);
make_semantic_handler(fun_def);
make_semantic_handler(return);
make_semantic_handler(if);
make_semantic_handler(if_else);
make_semantic_handler(while);
make_semantic_handler(compst);

make_arith_handler(assign);
make_arith_handler(id);
make_arith_handler(arith);
make_arith_handler(int);
make_arith_handler(float);
make_arith_handler(fun_call);
make_arith_handler(array_access);
make_arith_handler(struct_access);
make_arith_handler(uminus);
make_arith_handler(bool_to_int);

make_cond_handler(and);
make_cond_handler(or);
make_cond_handler(relop);
make_cond_handler(not);
make_cond_handler(int_to_bool);


#endif
