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

extern const operand op_zero;

typedef struct ir_ ir;
typedef void(*printer)(ir*);
struct ir_ {
    struct ir_ *prev, *next;
    printer func;
    operand op1, op2, res;
    union {
        const char* val_str;
        unsigned int val_int;
    };//may be needed for some information
};


struct label_ {
    unsigned cnt, no;
};
typedef struct label_* label;

void add_ir(ir*);
void print_ir();

label new_label(void);

operand new_temp_operand(void);
operand set_new_temp_operand(operand);
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
make_printer(fun_call);
make_printer(fun_dec);
make_printer(param);
#endif //__IR_H__
