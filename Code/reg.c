#include "common.h"
#include "table.h"
#include "reg.h"
#include "ir.h"

#define MAX_AVAIL 26
//MAX_AVAIL, MAX_AVAIL+1, ..., REG_NUMS - 1
//are reserved for other uses.
#define REG_NUMS 32

static operand reg_info[REG_NUMS] = {NULL};
static int is_using[REG_NUMS] = {0};

int output(const char* const fmt, ...);

static void load(operand op, int reg_idx) {
    if (op->kind == CONSTANT) {
        output("  li $%d, %d\n", reg_idx, op->val_int);
    } else {
        if(op->kind == ADDRESS) {
            op = op->op;
            const char *str = op_to_str(op);
            unsigned offset = table_lookup(str)->offset;
            free((void *)str);
            output("  addi $%d, $sp, %d\n", reg_idx, offset);
        } else {
            const char *str = op_to_str(op);
            Type t = table_lookup(str);
            free((void *)str);
            if(t->kind == OFFSET_BASIC) {
                output("  lw $%d, %d($sp)\n", reg_idx, t->offset);
            } else {
                output("  addi $%d, $sp, %d\n", reg_idx, t->offset);
            }
        }
        reg_info[reg_idx] = op;
    }
}

static void spill(int reg_idx) {
    if (reg_info[reg_idx] && reg_info[reg_idx]->kind != CONSTANT) {
        const char *str = op_to_str(reg_info[reg_idx]);
        unsigned offset = table_lookup(str)->offset;
        free((void *)str);
        output("  sw $%d, %d($sp)\n", reg_idx, offset);
    }
    reg_info[reg_idx] = NULL;
}

int find_op(operand op) {
    for(int i = 2; i < MAX_AVAIL; ++i) {
        if(!opcmp(op, reg_info[i])) {
            return i;
        }
    }
    return 0;
}

void reg_free(int reg_idx) {
    is_using[reg_idx] = 0;
    spill(reg_idx);
}
void op_free(operand op) {
    if(op->kind == ADDRESS) op = op -> op;
    reg_free(find_op(op));
}

int ensure(operand op) {
    if(op->kind == CONSTANT) return 0;
    return reg(op);
}

static int reg_real(operand op_raw, int load_flag) {
    operand op = op_raw;
    if(op && op->kind == ADDRESS) op = op -> op;
    int idx = find_op(op);
    if(idx) {
        is_using[idx] = 1;
        return idx;
    }
    for(int i = 2; i < MAX_AVAIL; ++i) {
        if(!is_using[i]) {
            if(load_flag) {
                load(op_raw, i);
            }
            reg_info[i] = op;
            is_using[i] = 1;
            return i;
        }
    }
    Assert(0);
    return 0;
}
int reg(operand op) {
    return reg_real(op, 1);
}
int tmp_reg() {
    return reg_real(NULL, 0);
}
int reg_noload(operand op) {
    Assert(op->kind != CONSTANT);
    return reg_real(op, 0);
}

void reg_init() {
    is_using[0] = is_using[1] = is_using[26] = is_using[27] = 1;
}

void reg_use(int reg_idx) {
    if(is_using[reg_idx] && reg_info[reg_idx]) {
        spill(reg_idx);
    }
    is_using[reg_idx] = 1;
    reg_info[reg_idx] = NULL;
}