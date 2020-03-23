#ifndef __TYPE_H__
#define __TYPE_H__

int strcmp(const char*, const char*);
#define IS(unit) (!strcmp(cur -> name, unit))

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

Type type_int;
Type type_float;

struct Type_ {
    enum {BASIC, ARRAY, STRUCT, FUNCTION, NOTYPE} kind;
    union {
        enum {INT, FLOAT} basic;
        struct {
            Type elem;
            int size;
        } array;
        FieldList structure;
        //function used this as well
        //return type -> arg1 -> arg2 -> ...
    };
};

struct FieldList_ {
    char* name;
    Type type;
    FieldList next;
};

Type to_array(Type type, int size);
Type to_struct(int cnt, ...);
Type to_func(Type ret_val, int cnt, ...);
Type get_type(node*);
int typecmp(Type t1, Type t2);
void type_print(Type t);

#endif
