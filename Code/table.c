#include <string.h>
#include "common.h"
#include "table.h"

#define LEN 12
#define SIZE (1<<LEN)

typedef struct tab tab;
struct tab{
    enum {TYPE, VARI} kind;
    Type type;
    const char* name;
    tab* next;
    int depth;
};
static int depth = 0;

static tab* table[SIZE] = {};
static tab* table_tail[SIZE] = {};

void init_hash_table() {
    static tab sentries[SIZE];
    for(int i = 0; i < SIZE; ++i) {
        table[i] = table_tail[i] = &(sentries[i]);
    }
}

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

struct LNode {
    tab* pre;
    struct LNode* next;
};

struct OLNode {
    //Orthogonal linked list
    struct OLNode* pre;// Head of OLList of depth-1
    struct LNode* dec;// Head of declarations of this OLList
};

struct OLNode* cur_list = NULL;

void new_scope() {
    struct OLNode* next_list = new(struct OLNode);
    next_list -> pre = cur_list;
    next_list -> dec = NULL;
    cur_list = next_list;
    ++depth;
}

void free_scope() {
    for(struct LNode* cur = cur_list -> dec; cur; cur = cur -> next) {
        tab* to_rm = cur -> pre -> next;
        cur -> pre -> next = to_rm -> next;
        if(to_rm -> next == NULL) {
            table_tail[hash(to_rm -> name)] = cur -> pre;
        }
        free(to_rm);
    }
    cur_list = cur_list -> pre;
    --depth;
}

static int table_insert_real(const char* name, Type type, int _depth) {
    my_log("Inserting %s\n", name);
    int x = hash(name);
    tab* p = new(tab);
    p -> type = type;
    p -> name = name;
    p -> next = NULL;
    p -> depth = _depth;

    table_tail[x] -> next = p;
    table_tail[x] = p;
    return 0;
}

int table_insert_global(const char* name, Type type) {
    //name will not be copied
    if(table_lookup(name)) return -1;
    return table_insert_real(name, type, 0);
}

int table_insert(const char* name, Type type) {
    //name will not be copied
    if(table_lookup(name)) return -1;
    if(depth > 0) {
        struct LNode* tmp = new(struct LNode);
        tmp -> next = cur_list -> dec;
        tmp -> pre = table_tail[hash(name)];
        cur_list -> dec = tmp;
    }
    return table_insert_real(name, type, depth);
}

Type table_lookup(const char* name) {
    int x = hash(name);
    tab* ret = NULL;
    for(tab* p = table[x] -> next;p; p = p -> next) {
        if(!strcmp(p -> name, name)) {
            ret = p;
        }
    }
    if(ret) {
        return ret -> type;
    } else {
        return NULL;
    }
}
