#include "type.h"
#include "table.h"
#include <stdio.h>
#include <assert.h>

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
    assert(typecmp(func1, func2));
    return 0;
}
