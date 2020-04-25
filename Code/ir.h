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

extern const operand op_zero, op_one;

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
    operand op1, op2;
    union {
        operand res;
        label l;
    };
    union {
        const char* val_str;
        unsigned int val_int;
    };//may be needed for more information
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

#define make_printer(name) \
    void name##_printer(ir* i)

make_printer(return);
make_printer(assign);
make_printer(binary);
make_printer(function);
make_printer(write);
make_printer(read);
make_printer(fun_call);
make_printer(fun_dec);
make_printer(param);
make_printer(label);
make_printer(goto);
make_printer(if_goto);
make_printer(if_nz);

make_printer(arg);
make_printer(dec);
make_printer(struct_dec);
make_printer(deref);

void tot_optimize();
#endif //__IR_H__
