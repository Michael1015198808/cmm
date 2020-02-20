#include "common.h"

void preorder(node*);
int error_cnt = 0;
int main(int argc, char** argv) {
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    if(!error_cnt) {
        preorder(root);
    }
    return 0;
}

int cmm_error(error e) {
    ++error_cnt;
    printf("Error type %c at Line %d: %s\n", e.type, e.lineno, e.msg);
}

int yyerror(char* msg) {
    error e;
    e.type = 'B';
    e.lineno = yylineno;
    e.msg = msg;
    cmm_error(e);
}

void preorder(node* cur) {
    static int indent = 0;
    if(indent)
        printf("%*c", indent, ' ');
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
