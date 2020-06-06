#ifndef __TYPE_H__
#define __TYPE_H__

#include "error.h"
#include "list.h"

int strcmp(const char*, const char*);

typedef struct Type_* Type;
typedef const struct Type_ * const CType;

typedef LIST_START(FieldList_)
    char* name;
    Type type;
LIST_END *FieldList;

extern CType type_int, type_float;

struct Type_ {
    unsigned size;
    enum {BASIC, ARRAY, STRUCTURE, STRUCTURE_DEF, FUNCTION, OFFSET} kind;
    union {
        enum {T_INT, T_FLOAT} basic;
        struct {
            Type elem;
            int size;
        } array;
        Type variable;
        struct {
            //function used this as well
            //return type -> arg1 -> arg2 -> ...
            FieldList structure;
            int is_dec:1, has_read:1, has_goto:1;
        };
        unsigned offset;
    };
};


Type to_array(CType type, unsigned size);
Type to_struct(int cnt, ...);
Type to_func(CType ret_val, int cnt, ...);

int typecmp(CType t1, CType t2);
Type type_check(CType lhs, CType rhs, CType ret, int lineno, semantic_errors err, ...);

void type_print(Type t);
void type_clear(Type t);

#endif
