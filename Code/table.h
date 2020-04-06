#ifndef __TABLE_H__
#define __TABLE_H__

#include "type.h"

void init_hash_table();
int table_insert_global(const char*,Type);
int table_insert(const char*,Type);
Type table_lookup(const char*);

void new_scope();
void free_scope();

#endif
