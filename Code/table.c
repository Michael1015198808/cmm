#include <string.h>
#include "common.h"
#include "table.h"

#define LEN 12
#define SIZE (1<<LEN)

typedef struct tab tab;
struct tab{
    enum {TYPE, VARI} kind;
    Type type;
    char* name;
    tab* next;
};
static tab* table[SIZE] = {};

int hash(const char* name) {
    unsigned int val = 0, i;
    for(; *name; ++name) {
        val = (val << 2) + *name;
        if((i = val & ~(SIZE - 1))) {
            val = (val ^ (i >> LEN)) & (SIZE - 1);
        }
    }
    return val;
}

int table_insert(const char* name, Type type) {
    my_log("Inserting %s\n", name);
    int x = hash(name);
    tab* p = malloc(sizeof(tab));
    p -> type = type;
    asprintf(&p -> name, "%s", name);
    p -> next = table[x];
    table[x] = p;
    return 0;
}

Type table_lookup(const char* name) {
    int x = hash(name);
    tab* p = table[x];
    while(p && strcmp(p -> name, name)) {
        p = p -> next;
    }
    if(p) {
        return p -> type;
    } else {
        return NULL;
    }
}
