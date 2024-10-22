#ifndef __IR_H__
#define __IR_H__
#include "type.h"
#include "list.h"
//operand, ir and label

typedef struct operand_ *operand;
struct operand_ {
    union {
        int t_no;
        int val_int;
        const char* val_str;
        operand op;
    };
    unsigned multi_use:1, checked:1;
    enum {
#define X(a, b, ...) \
        b,
#include "operand_kinds.def"
#undef X
    } kind;
};

const char* op_to_str(operand);

typedef struct ir_ ir;
typedef void(*printer)(ir*);

int opcmp(operand op1, operand op2);

typedef struct label_* label;
struct label_ {
    label parent;
    unsigned cnt, no;
    unsigned visit;
};
typedef struct {
    printer ir_format, mips_format;
} ir_ops;


#define make_ir_ops(name) \
    static ir_ops name##_ops_real = { \
        .ir_format = name##_ir_printer, \
        .mips_format = name##_mips_printer \
    }, *name##_ops = &name##_ops_real

#define make_ir_printer(name) \
    void name##_ir_printer(ir* i)

#define make_mips_printer(name) \
    void name##_mips_printer(ir* i)

make_ir_printer(return);
struct ir_ {
    struct ir_ *prev, *next;
    ir_ops* funcs;
    union {
        struct {
            operand op1, op2, res;
        };
        operand ops[3];
    };
    label l;
    const char* val_str;
    unsigned int val_int;
    //may be needed for more information
};


void print_ir();
void print_mips();
const ir* last_ir();

label new_label(void);
void label_add_true(label, operand);
void label_add_false(label, operand);
void print_label(label);
void print_label_goto(label);

#define X(a, b, ...) \
        operand new_##a##_operand(__VA_ARGS__); \
        operand set_##a##_operand(operand,##__VA_ARGS__);
#include "operand_kinds.def"
#undef X

void remove_ir(ir* i);
void  add_return_ir(operand op);
void* add_assign_ir(operand to, operand from);
void  add_arith_ir(operand to, operand lhs, int arith_op, operand rhs);
void  add_write_ir(operand op);
void  add_read_ir(operand op);
void  add_fun_call_ir(const char* name, operand op);
void  add_fun_dec_ir(const char* name);
void  add_param_ir_buffered(const char* name);
void  add_param_ir_flush();
void  add_if_goto_ir(operand op1, operand op2, const char* cmp, label l);
void  add_if_nz_ir(operand op1, label l);

void  add_arg_ir(operand op);
void  add_dec_ir(const char* name, unsigned size);

void tot_optimize();

void register_printf_operand();

#endif //__IR_H__
