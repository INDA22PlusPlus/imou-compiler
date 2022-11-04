#pragma once

#include <stdint.h>

#include "parser/parser.h"

#define __REALLOC_SYMBOL_TABLE_SIZE 32


typedef struct {
    char** vars;
    int32_t* values;
    
    uint8_t size;
    uint8_t capacity;
} _symbol_table;

/* Initializes an empty symbol table in the heap */
_symbol_table* _symbol_table_new();
/* Frees the symbol table from the heap*/
void _symbol_table_free(_symbol_table* table);

/* Pushes the new symbol to the table */
void _symbol_table_push(_symbol_table* table, char* var, int32_t value);

/*  Notice there is no `pop` function. On each new sub branch,
    the table is being copied and then unallocated when exiting */

/*  Returns the index of the var if in table else raises an error.
    We use the fact that all types are of DWORD and the pointer in
    the stack of the compiled program is 4+4*index */
uint8_t _symbol_table_get_ind(_symbol_table* table, char* var);


void _compile(ASTNode* root, _symbol_table* table);
