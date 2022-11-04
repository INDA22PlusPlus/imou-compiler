#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "compiler/compiler.h"
#include "parser/parser.h"


_symbol_table* _symbol_table_new() {
    _symbol_table* table = malloc(sizeof(table));
    if (!table) {
        perror("Unable to create a new symbol table");
        abort();
    }

    table->capacity = __REALLOC_SYMBOL_TABLE_SIZE;
    table->size     = 0;
    table->vars     = malloc(sizeof(char*) * __REALLOC_SYMBOL_TABLE_SIZE);
    table->values   = malloc(sizeof(int32_t*) * __REALLOC_SYMBOL_TABLE_SIZE);

    return table;
}

void _symbol_table_free(_symbol_table* table) {
    free(table->vars);
    free(table->values);
}

void _symbol_table_push(_symbol_table* table, char* var, int32_t value) {
    /* Extend capacity if needed*/
    if (table->capacity >= table->size) {
        table->capacity += __REALLOC_SYMBOL_TABLE_SIZE;
        table->vars     = realloc(table->vars, table->capacity);
        table->values   = realloc(table->values, table->capacity);
    }

    table->vars[table->size]    = var;
    table->values[table->size]  = value;

    table->size++; 
}

uint8_t _symbol_table_get_ind(_symbol_table* table, char* var) {
    /* Linear list search :( */
    for (uint8_t i = 0; i < table->size; i++) {
        if (strcmp(var, table->vars[i]) == 0) {
            return i;
        }
    }

    perror("Symbol not found in symbol table");
    abort();
}

/* Checks if the given node is of mathematical operator type */
static bool _node_is_mop(ASTNode* node) {
    switch (node->type)
    {
    case OPERATOR_DIV:      return true;
    case OPERATOR_MINUS:    return true;
    case OPERATOR_MUL:      return true;
    case OPERATOR_PLUS:     return true;
    default:                return false;
    }
}

static int _node_depth(ASTNode* node) {
    if (!node) { return 0; }

    if (node->children_num != 2) { return 1; }

    int l = _node_depth(&node->children[0]);
    int r = _node_depth(&node->children[1]);


    return (1 + ((l > r) ? l : r)); 
}

/* Writes the right asm instructions to load a constant or a variable from the table */
static void _compile_value_asm64(ASTNode* node, _symbol_table* table, const char* reg) {
    if (!node) {
        perror("Unexpected compiler error, `_compile_value_asm64`");
        abort();
    }

    if (node->type == CONSTANT) {
        printf("movl $%s, %%%s\n", node->value, reg);
        return;
    }

    uint8_t ind = _symbol_table_get_ind(table, node->value);
    printf("movl -%d(%%rbp), %%%s\n", 4*(ind+1), reg);
}


/*  `lor` => 0 = left node; 1 = right node. Indicates in which register
    r8 or 10 the answer should be loaded in. It is essential for parsing
    binary math expression trees*/
static void _compile_mexpr_asm64(ASTNode* root, _symbol_table* table, uint8_t lor) {
    const char* _reg = lor ? "r10d" : "r8d";

    if (root->children_num != 2) { return; }

    bool l_is_mop = _node_is_mop(&root->children[0]);
    bool r_is_mop = _node_is_mop(&root->children[1]);


    /* Fix this spaghetti code */
    if (l_is_mop && !r_is_mop) {
        _compile_mexpr_asm64(&root->children[0], table, 0);
        _compile_value_asm64(&root->children[1], table, "r10d");
    } else if (!l_is_mop && r_is_mop) {
        _compile_mexpr_asm64(&root->children[1], table, 1);
        _compile_value_asm64(&root->children[0], table, "r8d");
    } else if (l_is_mop && r_is_mop) {
        int l = _node_depth(&root->children[0]);
        int r = _node_depth(&root->children[1]);
        if (l > r) {
            _compile_mexpr_asm64(&root->children[0], table, 0);
            printf("movl %%r8d, %%eax\n");
            printf("pushq %%rax\n");
            _compile_mexpr_asm64(&root->children[1], table, 1);
            printf("popq %%rax\n");
            printf("movl %%eax, %%r8d\n");

        } else {
            _compile_mexpr_asm64(&root->children[1], table, 1);
            printf("movl %%r10d, %%eax\n");
            printf("pushq %%rax\n");
            _compile_mexpr_asm64(&root->children[0], table, 0);
            printf("popq %%rax\n");
            printf("movl %%eax, %%r10d\n");

        }

    } else {
        _compile_value_asm64(&root->children[0], table, "r8d");
        _compile_value_asm64(&root->children[1], table, "r10d");
    }

    switch (root->type)
    {
    case OPERATOR_PLUS:
        printf("xor %%r9d, %%r9d\n");
        printf("movl %%r8d, %%r9d\n");
        printf("addl %%r10d, %%r9d\n");
        printf("movl %%r9d, %%%s\n", _reg);
        break;
    case OPERATOR_MINUS:
        printf("xor %%r9d, %%r9d\n");
        printf("movl %%r10d, %%r9d\n");
        printf("subl %%r8d, %%r9d\n");
        printf("movl %%r9d, %%%s\n", _reg);
        break;
    case OPERATOR_MUL:
        printf("mov %%r8d, %%eax\n");
        printf("mul %%r10d\n");
        printf("mov %%eax, %%%s\n", _reg);
        break;
    case OPERATOR_DIV:
        printf("mov %%r10d, %%eax\n");
        printf("xor %%edx, %%edx\n");
        printf("div %%r8d\n");
        printf("mov %%eax, %%%s\n", _reg);
        break;
    default:
        perror("Unexcepted compiler error");
        abort();
    }
}

void _compile(ASTNode* root, _symbol_table* table) {
    if (!table) {
        table = _symbol_table_new();
    }

    if (root->children_num == 0) {
        _symbol_table_free(table);
        return;
    }

    for (int i = 0; i < root->children_num; i++) {

        switch (root->children[i].type)
        {
        case ASSIGNMENT:
            if (root->children[i].children_num != 2) {
                perror("Compiler error: assignment missing L or R value");
                abort();
            }
            _compile_mexpr_asm64(&root->children[i].children[1], table, 1);

            break;
        default: return;
        }
    }
}