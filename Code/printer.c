#include "handlers.h"

handler int_printer(node* node) {
    printf("INT: %d\n", node -> val_int);
}

handler float_printer(node* node) {
    printf("FLOAT: %lf\n", node -> val_float);
}

handler type_printer(node* node) {
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

handler id_printer(node* node) {
    printf("ID: %s\n", node -> val_str);
}
