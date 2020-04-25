#include "common.h"
#include "error.h"
#include "table.h"
#include "handlers.h"
#include <assert.h>

#include <mcheck.h>
int error_cnt;
int yydebug = 0;
FILE* out_file = NULL;

int optimization_level = 0;

void fun_dec_checker();
void free_tree(node* cur);

int main(int argc, char** argv) {
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
        if(argv[2]) {
            if(!(out_file = fopen(argv[2], "w"))) {
                perror(argv[2]);
                return 1;
            }
        }
    }
    error_cnt = 0;
    if(yyparse()) {
        syntax_error(yylineno, "End Of File unsupposed!");
    } else {
        if(!error_cnt) {
            //mtrace();

            init_hash_table();
            semantic(root);
            fun_dec_checker();
            if(!error_cnt) {
                tot_optimize();
                print_ir();
            }

            //table_clear();
            //remove_anonymous_struct();
            //muntrace();

            //free_tree(root);
        }
    }
    return 0;
}

void free_tree(node* cur) {
    for(int i = 0; i < cur->cnt; ++i) {
        if(cur->siblings[i])
            free_tree(cur->siblings[i]);
    }
    if(cur -> cnt > 0)
        free(cur -> siblings);
    if(!strcmp(cur -> name, "ID"))
        free(cur -> val_str);
    free(cur);
}
