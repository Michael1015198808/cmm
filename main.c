#include <stdio.h>

extern FILE* yyin;
int yylex();
int yyparse();

int main(int argc, char** argv) {
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    return 0;
}

int yyerror(char* msg) {
    return -1;
}
