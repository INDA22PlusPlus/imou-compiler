#pragma once
#include <stdlib.h>

#include "lexer.h"

#define __ASTNODE_REALLOC_SIZE 32


typedef enum {
    ASTNODETYPE_UNDEF,

    FUNCTION_CALL,
    FUNCTION_DEF,

    VARIABLE,
    CONSTANT,

    ASSIGNMENT,
    EXPRESSION,
    COMPARE,
    BRANCH,
    IF_BRANCH,
    ELSE_BRANCH,
    ELSEIF_BRANCH,
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

/*  Serializes the `ASTNode` tree from a given root, into json and puts
    it in stdout*/
void _stdout_json_serialize_ASTNode(ASTNode* root);

/*  The famous shunting yard algorithm used to convert infix math expressions
    to postfix aka reverse polish notation.                                 */
void _shunting_yard_alg(_token* token, _token** query, uint8_t* query_size,
                        _token** op_stack, uint8_t* op_stack_size,
                         _token_type end_symbol);
/*  Forces to pop the operators from the stack to the query. Should be used
    when end of a math expression has occured                               */
void _shunting_yard_alg_force_clean(_token** query, uint8_t* query_size,
                                    _token** op_stack, uint8_t* op_stack_size);

/* Parses reverse polish notation of mathematical expressions */
ASTNode* _parse_rpn(_token** query, uint8_t *query_size);