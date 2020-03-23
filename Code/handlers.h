#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include"common.h"

void int_printer(node*);
void float_printer(node*);
void type_printer(node*);
void id_printer(node*);

const char* get_vardec_name(node*);
void def_handler(node*);
#endif
