#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define YYSTYPE node*

extern FILE* yyin;
int yylineno;
int yylex();
int yyparse();
int yyerror(char* msg);

int asprintf(char**, const char*, ...);
int vasprintf(char**, const char*, va_list);

typedef struct node node;
typedef void(*handler)(node*);

struct node {
    handler func;
    const char* name;
    int cnt, lineno;
    union {
        struct node** siblings;
        const char* val_str;
        unsigned int val_int;
        double val_float;
    };
};

node* root;

static inline node* Node(const char* name,int lineno, int cnt, ...) {
    node* ret = (node*)malloc(sizeof(node));
    ret->name = name;
    ret->lineno = lineno;
    ret->cnt = cnt;
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

int lexical_error(int lineno, const char* fmt, ...);
int syntax_error(int lineno, const char* fmt, ...);

#define TODO() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#define Assert() \
    do { \
        printf("%s: %d not implemented\n", __FILE__, __LINE__); \
    } while(0)

#endif
