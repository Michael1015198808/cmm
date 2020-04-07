#include "common.h"
#include "table.h"
#include <assert.h>

#include <mcheck.h>
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
            //mtrace();
            init_hash_table();
            preorder(root);
            void fun_dec_checker();
            fun_dec_checker();
            void free_tree(node* cur);
            free_tree(root);
            //muntrace();
        }
    }
    return 0;
}

int hash(const char*);
int hash_test() {
    printf("%d\n", hash("i"));
    printf("%d\n", hash("qwq"));
    printf("%d\n", hash("qaq"));
    printf("%d\n", hash("1015198808"));
    printf("%d\n", hash("171240518 Zhenyu Yan"));
    printf("%d\n", hash("1234567654321"));
    printf("%d\n", hash("a1234567654320"));
    printf("%d\n", hash("a2234567654321"));
    return 0;
}
int array_test() {
    Type arr1 = to_array(type_int, 10);
    Type arr2 = to_array(arr1, 5);
    Type arr3 = to_array(arr1, 10);
    Type arr4 = to_array(type_int, 10);

    type_print(arr1);
    type_print(arr2);
    type_print(arr3);
    type_print(arr4);

    assert(arr1 != arr4);
    assert(!typecmp(arr1, arr4));
    assert(typecmp(arr1, arr2));
    assert(typecmp(arr1, arr3));
    assert(typecmp(arr2, arr3));
    return 0;
}
int struct_test() {
    Type stru1 = to_struct(1, "i", type_int);
    Type stru2 = to_struct(3, "i1", type_int, "i2", type_int, "f1", type_float);
    Type arr1 = to_array(stru1, 5);

    type_print(stru1);
    type_print(arr1);
    type_print(stru2);

    Type stru3 = to_struct(1, "qwq", type_int);

    assert(!typecmp(stru1, stru3));
    assert(typecmp(stru1, stru2));
    assert(typecmp(stru2, stru3));
    return 0;
}
int func_test() {
    Type func1 = to_func(type_int, 0);
    Type func2 = to_func(type_int, 2, "i2", type_int, "f1", type_float);

    type_print(func1);
    type_print(func2);

    Type func3 = to_func(type_int, 0);

    assert(!typecmp(func1, func3));
    assert(typecmp(func1, func2));
    assert(typecmp(func2, func3));
    return 0;
}
/*
int main(int argc, char** argv) {
    //hash_test();
    //array_test();
    //struct_test();
    //func_test();
    return 0;
}
*/

void preorder(node* cur) {
    if(cur -> func) {
        cur -> func(cur);
    } else {
        for(int i = 0; i < cur->cnt; ++i) {
            if(cur->siblings[i])
                preorder(cur->siblings[i]);
        }
    }
}

void free_tree(node* cur) {
    for(int i = 0; i < cur->cnt; ++i) {
        if(cur->siblings[i])
            free_tree(cur->siblings[i]);
    }
    if(cur -> cnt > 0)
        free(cur -> siblings);
    free(cur);
}
