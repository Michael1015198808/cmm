#include "common.h"

void preorder(node*);
int main(int argc, char** argv) {
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    preorder(root);
    return 0;
}

int yyerror(char* msg) {
    return -1;
}
void preorder(node* cur) {
    static int indent = 0;
    if(indent)
        printf("%*c", indent, ' ');
    if(cur->cnt) {
        printf("%s (%d)\n", cur->name, cur->lineno);
    } else {
        printf("%s\n", cur->name);
    }
    indent+=2;
    for(int i = 0; i < cur->cnt; ++i) {
        if(cur->siblings[i])
            preorder(cur->siblings[i]);
    }
    indent-=2;
}
