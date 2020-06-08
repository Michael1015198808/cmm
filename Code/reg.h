#ifndef __REG_H__
#define __REG_H__

#include "reg.h"

int reg(operand op);
int reg_noload(operand op);
int tmp_reg();
void reg_use(int reg_idx);

void reg_free(int reg_idx);
void op_free(operand);

void reg_check();

#endif //__REG_H__
