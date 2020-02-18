#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define YYSTYPE node*

extern FILE* yyin;
int yylineno;
int yylex();
int yyparse();
int yyerror(char* msg);

int asprintf(char**, const char*, ...);
int vasprintf(char**, const char*, va_list);

typedef struct node {
    const char* name;
    int cnt, lineno;
    struct node** siblings;
} node;

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

typedef struct error {
    char* msg;
    int lineno;
    char type;
} error;
int cmm_error(error);
