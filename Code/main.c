#include "common.h"
#include "error.h"
#include "table.h"
#include "handlers.h"
#include <assert.h>

int error_cnt;
int yydebug = 0;

void fun_dec_checker();
void free_tree(node* cur);


int main(int argc, const char** argv) {
    void opt_parse(int argc, const char** argv);
    opt_parse(argc, argv);
    error_cnt = 0;
    if(yyparse()) {
        syntax_error(yylineno, "End Of File unsupposed!");
    } else {
        if(!error_cnt) {

            init_hash_table();
            semantic(root);
            fun_dec_checker();
            if(!error_cnt) {
                tot_optimize();
                register_printf_operand();
                //print_ir();
                print_mips();
            }

            //table_clear();
            //remove_anonymous_struct();
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
