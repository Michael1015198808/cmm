#ifndef __IR_H__
#define __IR_H__
#include "type.h"
//operand, ir and label

typedef struct operand_ *operand;
struct operand_ {
    enum {VARIABLE, CONSTANT, ADDRESS, POINTER, TEMP} kind;
    union {
        int t_no;
        int val_int;
        const char* val_str;
        operand op;
    };
};

extern const operand op_zero;

typedef struct ir_ ir;
typedef void(*printer)(ir*);

struct label_ {
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
void print_label(label);
void print_label_goto(label);

operand new_temp_operand(void);
operand set_temp_operand(operand);

operand new_const_operand(int num);
operand set_const_operand(operand, int num);

operand new_variable_operand(const char*);
operand set_variable_operand(operand, const char*);
#define INIT1(ir) \
    do { \
        ir -> op1 = op1; \
        ir -> res = res; \
    } while(0)
#define INIT2(ir) \
    do { \
        ir -> op1 = op1; \
        ir -> op2 = op2; \
        ir -> res = res; \
    } while(0)

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
make_printer(arg);
make_printer(dec);
make_printer(struct_dec);
make_printer(deref);
#endif //__IR_H__
