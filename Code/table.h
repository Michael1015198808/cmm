#ifndef __TABLE_H__
#define __TABLE_H__

#include "type.h"

int hash(const char* str);

void init_hash_table();
int table_insert_global(const char*,Type);
int table_insert_struct(const char*,Type);
int table_insert(const char*,Type);
Type table_lookup(const char*);
Type table_lookup_struct(const char*);
void table_print();
void table_clear();

void new_scope();
void free_scope();
void struct_free_scope();

void add_anonymous_struct(Type ret);
void remove_anonymous_struct();
#endif
