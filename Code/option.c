#include "common.h"

extern FILE* yyin;
FILE* out_file;
int optimization_level = 1;

void redirect_input(const char* file) {
    if(!(yyin = fopen(file, "r"))) {
        perror(file);
        exit(-1);
    }
}
void redirect_output(const char* file) {
    if(!(out_file = fopen(file, "w"))) {
        perror(file);
        exit(-1);
    }
}
void(*default_func[])(const char*) = {
    redirect_input,
    redirect_output,
    NULL,
};
void opt_parse(int argc, const char** argv) {
    for(int i = 1, j = 0; i < argc; ++i) {
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'O':
                    optimization_level = atoi(argv[i] + 2);
                    break;
                default:
                    err("Unrecognized option '%c'!\n", argv[i][1]);
                    exit(-1);
            }
        } else {
            if(default_func[j]) {
                default_func[j++](argv[i]);
            } else {
                err("Too many arguments!\n");
                exit(-1);
            }
        }
    }
}
