#include "common.h"

void int_printer(node* node) {
    printf("INT: %d\n", node -> val_int);
}

void float_printer(node* node) {
    printf("FLOAT: %lf\n", node -> val_float);
}

void type_printer(node* node) {
    printf("TYPE: ");
    switch(node -> val_int) {
        case 1:
            puts("int");
            break;
        case 2:
            puts("float");
            break;
        default:
            Assert();
    }
}

void id_printer(node* node) {
    printf("ID: %s\n", node -> val_str);
}
