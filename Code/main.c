#include "common.h"

void preorder(node*);
int error_cnt = 0;
int yydebug = 1;
int main(int argc, char** argv) {
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    if(yyparse()) {
        syntax_error(yylineno, "End Of File unsupposed!");
    } else {
        if(!error_cnt) {
            preorder(root);
        }
    }
    return 0;
}

int lexical_error(int lineno, const char* fmt, ...) {
    ++error_cnt;
    va_list ap;
    va_start(ap, fmt);
    printf("Error type A at Line %d: ", lineno);
    vprintf(fmt, ap);
    putchar('\n');
}

int syntax_error(int lineno, const char* fmt, ...) {
    static int last_line = 0;
    if(lineno == last_line) {
        return -1;//Prevent reporting too much errors
    }
    last_line = lineno;
    ++error_cnt;
    va_list ap;
    va_start(ap, fmt);
    printf("Error type B at Line %d: ", lineno);
    vprintf(fmt, ap);
    putchar('\n');
}

void preorder(node* cur) {
    static int indent = 0;
    printf("%*s", indent, "");
    if(cur->cnt) {
        printf("%s (%d)\n", cur->name, cur->lineno);
        indent+=2;
        for(int i = 0; i < cur->cnt; ++i) {
            if(cur->siblings[i])
                preorder(cur->siblings[i]);
        }
        indent-=2;
    } else {
        if(cur -> func) {
            cur -> func(cur);
        } else {
            puts(cur->name);
        }
    }
}
