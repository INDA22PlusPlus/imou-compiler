#pragma once

#include <stdlib.h>

#include "lexer.h"

#define __ASTNODE_REALLOC_SIZE 32


typedef enum {
    ASTNODETYPE_UNDEF,

    VARIABLE,
    CONSTANT,

    ASSIGNMENT,
    EXPRESSION,
    COMPARE,
    BRANCH,
    IF_BRANCH,
    WHILE_BRANCH,
    FOR_BRANCH,

    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_DIV,
    OPERATOR_MUL,
}   _ASTNodeType;

struct _ASTNode {
    struct _ASTNode* parent;
    struct _ASTNode* children;

    uint8_t children_num;
    uint8_t capacity;

    _ASTNodeType type;
    char* value;

    char** options;
};

typedef struct _ASTNode ASTNode; 

/* Parses the tokens array and returns the AST root node */
ASTNode* _parse(_token* tokens, size_t token_num);

/*  Initializes an empty ASTNode with NULL values and `BRANCH` type,
    just to clean up the code                                        */
ASTNode* _init_default_ASTNode();

/* Pushes the given child to the parent. Handles capacity management */
void _push_child(ASTNode* parent, ASTNode* child);

/*  Tries to construct a variable `ASTNode` from the given
    stack                                                            */
void _build_var_from_stack(ASTNode* node, _token** stack, uint8_t stack_size);

void _shunting_yard_alg(_token* token, _token** query, uint8_t* query_size,
                        _token** op_stack, uint8_t* op_stack_size,
                         _token_type end_symbol);

_ASTNodeType _token_type_to_node_type(_token_type type);

ASTNode* _parse_rpn(_token** query, uint8_t query_size);