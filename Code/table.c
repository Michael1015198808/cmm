#include <string.h>
#include <limits.h>
#include "common.h"
#include "table.h"
#include "type.h"

#define LEN 14
#define SIZE (1<<LEN)

typedef struct tab tab;
struct tab{
    Type type;
    const char* name;
    tab* next;
    unsigned depth;
};
static unsigned depth;

static tab* table[SIZE] = {NULL};
static tab* table_tail[SIZE] = {NULL};

void init_hash_table() {
    static tab sentries[SIZE];
    depth = 0;
    for(int i = 0; i < SIZE; ++i) {
        table[i] = table_tail[i] = &(sentries[i]);
    }
    table_insert("read", to_func(type_int, 0));
    table_insert("write", to_func(type_int, 1, "", type_int));
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

static inline void free_scope_real(int to_free) {
    for(struct LNode* cur = cur_list -> dec; cur; cur = remove_access(cur, next)) {
        tab* to_rm = cur -> pre -> next;
        if(!to_rm -> type -> r_val) {
            remove_unused_variable(to_rm->name);
        }
        if(to_rm -> next == NULL) {
            table_tail[hash(to_rm -> name)] = cur -> pre;
        }
        if(to_free && cur -> pre -> next -> type)
            type_clear(cur -> pre -> next -> type);
        cur -> pre -> next = remove_access(to_rm, next);
    }
    cur_list = remove_access(cur_list, pre);
    --depth;
}
void struct_free_scope() {
    free_scope_real(0);
}
void free_scope() {
    free_scope_real(1);
}

static int table_insert_real(const char* name, Type type, unsigned _depth) {
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

static Type table_lookup_all(const char* name, int depth);

int table_insert_struct(const char* name, Type type) {
    if(table_lookup_all(name, 0)) return -1;
    return table_insert_real(name, type, UINT_MAX);
}

int table_insert_global(const char* name, Type type) {
    if(table_lookup_all(name, 0)) return -1;
    return table_insert_real(name, type, 0);
}

int table_insert(const char* name, Type type) {
    if(table_lookup_all(name, depth)) return -1;
    if(depth > 0) {
        struct LNode* tmp = new(struct LNode);
        tmp -> next = cur_list -> dec;
        tmp -> pre = table_tail[hash(name)];
        cur_list -> dec = tmp;
    }
    return table_insert_real(name, type, depth);
}

static Type table_lookup_all(const char* name, int depth) {
    int x = hash(name);
    tab* ret = NULL;
    for(tab* p = table[x] -> next;p; p = p -> next) {
        if(p -> depth >= depth && !strcmp(p -> name, name)) {
            ret = p;
        }
    }
    if(ret) {
        return ret -> type;
    } else {
        return NULL;
    }
}

Type table_lookup_variable(const char* name, int depth) {
    int x = hash(name);
    tab* ret = NULL;
    for(tab* p = table[x] -> next;p; p = p -> next) {
        if(p -> depth >= depth && !strcmp(p -> name, name) && p -> type -> kind != STRUCTURE_DEF) {
            ret = p;
        }
    }
    if(ret) {
        return ret -> type;
    } else {
        return NULL;
    }
}

Type table_lookup(const char* name) {
    return table_lookup_variable(name, 0);
}

Type table_lookup_struct(const char* name) {
    return table_lookup_all(name, 0);
}

void type_clear(Type);

void table_print() {
    for(int i = 0; i < SIZE; ++i) {
        if(table[i] != table_tail[i]) {
            for(tab* cur = table[i] -> next; ; cur = cur -> next) {
                printf("\n%s(%d): ", cur -> name, cur -> depth);
                type_print(cur -> type);
                if(cur == table_tail[i]) {
                    break;
                }
            }
        }
    }
}
void table_clear() {
    for(int i = 0; i < SIZE; ++i) {
        if(table[i] != table_tail[i]) {
            for(tab* cur = table[i] -> next; ; cur = remove_access(cur, next)) {
                type_clear(cur -> type);
                if(cur == table_tail[i]) {
                    free(cur);
                    break;
                }
            }
        }
    }
}

static struct struct_dec_list {
    Type type;
    struct struct_dec_list* next;
} *struct_dec_list_head = NULL;

void add_anonymous_struct(Type ret) {
    struct struct_dec_list* tmp = new(struct struct_dec_list);
    tmp -> type = ret;
    tmp -> next = struct_dec_list_head;
    struct_dec_list_head = tmp;
}

void remove_anonymous_struct() {
    for(struct struct_dec_list* cur = struct_dec_list_head; cur; cur = remove_access(cur, next)) {
        type_clear(cur -> type);
    }
}
