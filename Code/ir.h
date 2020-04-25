#ifndef __IR_H__
#define __IR_H__
#include "type.h"
#include "list.h"
//operand, ir and label

typedef struct operand_ *operand;
struct operand_ {
    enum {VARIABLE, CONSTANT, ADDRESS, POINTER, TEMP, NONE} kind;
    union {
        int t_no;
        int val_int;
        const char* val_str;
        operand op;
    };
};

typedef struct ir_ ir;
typedef void(*printer)(ir*);


struct label_ {
    LIST(var, operand) *tlist, *flist;
    unsigned cnt, no;
};
typedef struct label_* label;

struct ir_ {
    struct ir_ *prev, *next;
    printer func;
    operand op1, op2, res;
    label l;
    const char* val_str;
    unsigned int val_int;
    //may be needed for more information
};


void add_ir(ir*);
void print_ir();

label new_label(void);
void label_add_true(label, operand);
void label_add_false(label, operand);
void print_label(label);
void print_label_goto(label);

operand new_temp_operand(void);
operand set_temp_operand(operand);

operand new_const_operand(int num);
operand set_const_operand(operand, int num);

operand new_variable_operand(const char*);
operand set_variable_operand(operand, const char*);

void add_return_ir(operand op);
void add_assign_ir(operand to, operand from);
void add_arith_ir(operand to, operand lhs, int arith_op, operand rhs);
void add_write_ir(operand op);
void add_read_ir(operand op);
void add_fun_call_ir(const char* name, operand op);
void add_fun_dec_ir(const char* name);
void add_param_ir(const char* name);
void add_if_goto_ir(operand op1, operand op2, const char* cmp, label l);
void add_if_nz_ir(operand op1, label l);

void add_arg_ir(operand op);
void add_dec_ir(const char* name, unsigned size);

void tot_optimize();
#endif //__IR_H__
