#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ir.h"

#define YYSTYPE node*


#define new(type) (type*)malloc(sizeof(type))

static inline void* free_first(void* ptr1, void* ptr2) {
    free(ptr1);
    return ptr2;
}

#define remove_access(vari, field) \
    free_first(vari, vari -> field)

extern FILE* yyin;
int yylineno;
int yyparse();

typedef struct node node;
typedef void*(*handler)(node*, operand, label, label);

struct node {
    handler func;
    const char* name;
    int cnt, lineno;
    struct node** siblings;
    union {
        char* val_str;
        unsigned int val_int;
        float val_float;
    };
};

extern node* root;

static inline node* Node(const char* name,int lineno, int cnt, ...) {
    node* ret = (node*)malloc(sizeof(node));
    ret -> func = NULL;
    ret -> name = name;
    ret -> cnt = cnt;
    ret -> lineno = lineno;
    if(cnt) {
        ret -> siblings = (node**)malloc(sizeof(void*) * cnt);
        va_list ap;
        va_start(ap, cnt);
        for(int i = 0; i < cnt; ++i) {
            ret -> siblings[i] = va_arg(ap, node*);
        }
        va_end(ap);
    } else {
        ret -> siblings = NULL;
    }
    return ret;
}

static inline node* Singleton(const char* name) {
    extern int yylineno;
    return Node(name, yylineno, 0);
}

#ifdef LOCAL
#define TODO() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#define Assert(cond) \
    if(!(cond)) { \
        printf("%s: %d assertion %s failed!\n", __FILE__, __LINE__, #cond); \
        exit(-1); \
    } while(0)

#define panic() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#define IF(cond) \
    if(cond)

#else
#define TODO(...)
#define Assert(...)
#define panic(...)
#define IF(cond) \
    if((cond) && 0)
#endif
#endif // __COMMON_H__
