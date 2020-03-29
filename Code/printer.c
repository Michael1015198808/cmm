#include "common.h"

void* int_printer(node* node) {
    printf("INT: %u\n", node -> val_int);
    return NULL;
}

void* float_printer(node* node) {
    printf("FLOAT: %f\n", node -> val_float);
    return NULL;
}

void* type_printer(node* node) {
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
    return NULL;
}

void* id_printer(node* node) {
    printf("ID: %s\n", node -> val_str);
    return NULL;
}
