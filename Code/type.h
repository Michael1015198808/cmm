#ifndef __TYPE_H__
#define __TYPE_H__

#include "error.h"

int strcmp(const char*, const char*);
#define IS(unit) (!strcmp(cur -> name, unit))

typedef struct Type_* Type;
typedef const struct Type_ * const CType;
typedef struct FieldList_* FieldList;

extern CType type_int, type_float;

struct Type_ {
    enum {BASIC, ARRAY, STRUCTURE, STRUCTURE_DEF, FUNCTION, NOTYPE} kind;
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
            int is_dec;
        };
    };
};

struct FieldList_ {
    char* name;
    Type type;
    FieldList next;
};

Type to_array(CType type, int size);
Type to_struct(int cnt, ...);
Type to_func(CType ret_val, int cnt, ...);

int typecmp(CType t1, CType t2);
Type type_check(CType lhs, CType rhs, CType ret, int lineno, semantic_errors err, ...);

void type_print(Type t);
void type_clear(Type t);

#endif
