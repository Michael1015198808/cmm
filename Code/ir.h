#ifndef __IR_H__
#define __IR_H__
#include "type.h"
//operand, ir and label

struct operand_ {
    enum {VARIABLE, CONSTANT, ADDRESS, TEMP} kind;
    union {
        int t_no;
        int val_int;
        const char* val_str;
    };
    Type t;
};
typedef struct operand_ *operand;

struct ir_ {
    struct ir_ *prev, *next;
    struct {
        enum { ASSIGN, ADD, SUB, MUL  } kind;
        operand op1, op2, res;
    };
};
typedef struct ir_ ir;


struct label_ {
    unsigned cnt, no;
};
typedef struct label_* label;

void add_ir(ir*);
void print_ir();

label new_label(void);

operand new_temp_operand(void);
operand set_new_temp_operand(operand);
#endif //__IR_H__
