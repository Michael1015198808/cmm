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
            output("  lw $%d, 0($%d)\n", reg_idx, reg(op->op));
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
    }
}

static void spill(int reg_idx) {
    operand op = reg_info[reg_idx];
    if (op && op->kind != CONSTANT) {
        if(op->kind == ADDRESS) {
            int r1 = reg(op->op);
            output("  sw $%d, 0($%d)\n", reg_idx, r1);
        } else {
            const char *str = op_to_str(reg_info[reg_idx]);
            Type t = table_lookup(str);
            free((void *)str);
            if (t->kind == OFFSET_BASIC)
                output("  sw $%d, %d($sp)\n", reg_idx, t->offset);
        }
    }
    reg_info[reg_idx] = NULL;
}
void spill_all() {
    for(int i = 2; i < MAX_AVAIL; ++i) {
        spill(i);
    }
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
    spill(reg_idx);
    is_using[reg_idx] = 0;
    reg_info[reg_idx] = NULL;
}

void op_free(operand op) {
    reg_free(find_op(op));
}

static int reg_real(operand op, int load_flag) {
    int idx = find_op(op);
    if(idx) {
        is_using[idx] = 1;
        return idx;
    }
    for(int i = 2; i < MAX_AVAIL; ++i) {
        if(!is_using[i]) {
            if(load_flag) {
                load(op, i);
            }
            reg_info[i] = op;
            is_using[i] = 1;
            return i;
        }
    }
    Assert(0);
    return 0;
}

//return the register contains the op
int reg(operand op) {
    return reg_real(op, 1);
}
//return a register unused
int tmp_reg() {
    return reg_real(NULL, 0);
}
//mark a register with op
int reg_noload(operand op) {
    Assert(op->kind != CONSTANT);
    return reg_real(op, 0);
}

void reg_use(int reg_idx) {
    if(is_using[reg_idx] && reg_info[reg_idx]) {
        spill(reg_idx);
    }
    is_using[reg_idx] = 1;
    reg_info[reg_idx] = NULL;
}