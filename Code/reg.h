#ifndef __REG_H__
#define __REG_H__

#include "reg.h"

void op_free(operand);
int reg_noload(operand op);
int reg(operand op);
int ensure(operand op);
void reg_init();
void reg_use(int reg_idx);
int find_op(operand op);
void reg_free(int reg_idx);
int tmp_reg();

#endif //__REG_H__