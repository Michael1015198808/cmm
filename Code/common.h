#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"

#define YYSTYPE node*

#define new(type) (type*)malloc(sizeof(type));

int lexical_error(int lineno, const char* fmt, ...);
int syntax_error(int lineno, const char* fmt, ...);
int semantic_error(int lineno, int errorno, ...);
int vsemantic_error(int lineno, int errorno, va_list);

extern FILE* yyin;
int yylineno;
int yylex();
int yyparse();
int yyerror(char* msg);

int asprintf(char**, const char*, ...);
int vasprintf(char**, const char*, va_list);

typedef struct node node;
typedef void*(*handler)(node*);

struct node {
    handler func;
    const char* name;
    int cnt, lineno;
    union {
        struct node** siblings;
        char* val_str;
        unsigned int val_int;
        float val_float;
    };
};
#define foreach(name, init, next) \
    for(node* name = cur -> siblings[init];;name = name -> siblings[next])

node* root;

static inline node* Node(const char* name,int lineno, int cnt, ...) {
    node* ret = (node*)malloc(sizeof(node));
    ret->func = NULL;
    ret->name = name;
    ret->cnt = cnt;
    ret->lineno = lineno;
    ret->siblings = (node**)malloc(sizeof(void*) * cnt);
    va_list ap;
    va_start(ap, cnt);
    for(int i = 0; i < cnt; ++i) {
        ret -> siblings[i] = va_arg(ap, node*);
    }
    va_end(ap);
    return ret;
}

static inline node* Singleton(const char* fmt, ...) {
    char* buf;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(&buf, fmt, ap);
    extern int yylineno;
    return Node(buf, yylineno, 0);
}

void preorder(node* cur);

#define TODO() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#define Assert(cond) \
    if(!(cond)) { \
        printf("%s: %d assertion %s failed!\n", __FILE__, __LINE__, #cond); \
    } while(0)

#define panic() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#endif
