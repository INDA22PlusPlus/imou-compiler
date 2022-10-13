#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "lexer.h"


/* Takes a memory block and converts it to a null-terminated string*/
static char* memcpy_str(void* mem, size_t size) {
    char* _str = malloc(size + 1);
    _str[size] = '\0';
    memcpy(_str, mem, size);

    return _str;
}

ASTNode* _init_default_ASTNode() {
    ASTNode* target = malloc( sizeof(ASTNode) );
    
    target->parent          = NULL;
    target->children        = malloc(sizeof(ASTNode)*__ASTNODE_REALLOC_SIZE);
    target->children_num    = 0;
    target->type            = BRANCH;
    target->value           = NULL;
    target->options         = NULL;

    return target;
}

void _push_child(ASTNode* parent, ASTNode* child) {
    /* Expand the node's capacity if needed*/
    if (parent->capacity == parent->children_num) {
        parent->capacity += __ASTNODE_REALLOC_SIZE;
        parent->children = realloc(parent->children,
                                    sizeof(ASTNode)*parent->capacity);
    }

    /* Overwrite the parent if not equal */
    if (parent != child->parent) {child->parent = parent; }

    parent->children[parent->children_num++] = *child;
    free(child);
}

void _build_var_from_stack(ASTNode* node, _token** stack, uint8_t stack_size) {
    node->type          = VARIABLE;
    node->capacity      = 0;
    node->children      = NULL;
    node->children_num  = 0;

    /*  Simple check if the format is valid.
        `stack_size` == 1: already defined variable;
        `stack_size` == 4: new variable with given type*/
        
    /* When memory overflow without error is actually a good thing*/
    if ((stack_size != 1 || stack_size != 4) && 
        (stack[0]->type != TYPE_VALUE_VAR)) {
    
        perror("Not valid variable identifier");
        abort();
    }

    node->value = memcpy_str(stack[0]->beg, stack[0]->length);
    
    if (stack_size == 1) {return;}

    if (stack[1]->type != SYMBOL_OTRIANGLE ||
        stack[3]->type != SYMBOL_CTRIANGLE) {

        perror("Not valid variable identifier");
        abort();
    }

    node->options = malloc(sizeof(char*) * 1);
    node->options[0] = memcpy_str(stack[2]->beg, stack[2]->length);
}

ASTNode* _node_from_token(_token* token, _token** stack,
                                uint8_t* stack_size) {

    ASTNode*    target;
    

    target = _init_default_ASTNode();
    if (!_is_token_special(token) || token->type == SYMBOL_SEMICOLON) {
        return NULL;
    }


    switch (token->type)
    {
    case SYMBOL_OBRACE:
        target->type = BRANCH;
        break;
    case SYMBOL_OPARENTHESIS:
        target->type = EXPRESSION;
        break;
    case SYMBOL_EQUAL:
        target->type = ASSIGNMENT;
        
        _build_var_from_stack(&target->children[0], stack, stack_size);
        target->children_num++;

        *stack_size = 0;
        break;
    case SYMBOL_PLUS:
        target->type = OPERATOR_PLUS;

        if (stack_size != 1 || stack[0]->type == TYPE_VALUE_NUM
            || stack[0]->type == TYPE_VALUE_VAR) {
            perror("Non-valid expression");
            abort();
        }

        target->children[0].type = (stack[0]->type==TYPE_VALUE_VAR)?VARIABLE:CONSTANT;
        target->children[0].value = memcpy_str(stack[0]->beg, stack[0]->length);
        target->children_num++;

        *stack_size = 0;
        break;
    case SYMBOL_MINUS:
        target->type = OPERATOR_MINUS;

        if (stack_size != 1 || stack[0]->type == TYPE_VALUE_NUM
            || stack[0]->type == TYPE_VALUE_VAR) {
            perror("Non-valid expression");
            abort();
        }

        target->children[0].type = (stack[0]->type==TYPE_VALUE_VAR)?VARIABLE:CONSTANT;
        target->children[0].value = memcpy_str(stack[0]->beg, stack[0]->length);
        target->children_num++;

        *stack_size = 0;
        break;
    case SYMBOL_MUL:
        target->type = OPERATOR_MUL;

        if (*stack_size != 1 || stack[0]->type == TYPE_VALUE_NUM
            || stack[0]->type == TYPE_VALUE_VAR) {
            perror("Non-valid expression");
            abort();
        }

        target->children[0].type = (stack[0]->type==TYPE_VALUE_VAR)?VARIABLE:CONSTANT;
        target->children[0].value = memcpy_str(stack[0]->beg, stack[0]->length);
        target->children_num++;

        *stack_size = 0;
        break;
    case SYMBOL_DIVIDE:
        target->type = OPERATOR_DIV;

        if (stack_size != 1 || stack[0]->type == TYPE_VALUE_NUM
            || stack[0]->type == TYPE_VALUE_VAR) {
            perror("Non-valid expression");
            abort();
        }

        target->children[0].type = (stack[0]->type==TYPE_VALUE_VAR)?VARIABLE:CONSTANT;
        target->children[0].value = memcpy_str(stack[0]->beg, stack[0]->length);
        target->children_num++;

        *stack_size = 0;
        break;
    case KEYWORD_IF:
        target->type = IF_BRANCH;
        break;
    case KEYWORD_WHILE:
        target->type = WHILE_BRANCH;
        break;
    case KEYWORD_FOR:
        target->type = FOR_BRANCH;
        break;
    default:    return NULL;
    }
    return target;
}

void _shunting_yard_alg(_token* token, _token** query, uint8_t* query_size,
                        _token** op_stack, uint8_t* op_stack_size,
                         _token_type end_symbol) {

    /* The last operator on the stack */
    _token* op = op_stack[*op_stack_size-1];

    /* Empty the stack on expression end */
    if (token->type == end_symbol && *op_stack_size > 0) {
        for(int i = *op_stack_size-1; i >= 0; i--) {
            query[(*query_size)++] = op_stack[i];
        }
        
        *op_stack_size = 0;
    }

    if (token->type == TYPE_VALUE_NUM ||
            token->type == TYPE_VALUE_VAR) {
        /* Push the token to the query if a number of var */
        query[(*query_size)++] = token;
        return;
    }

    /* The precedence and associativity of the current token */
    uint8_t token_prec  = _token_precedence(token);
    uint8_t token_assoc = _token_associativity(token);
    if (*op_stack_size > 0) {
        for(int i = *op_stack_size-1; i >= 0; i--) {
            uint8_t op_prec = _token_precedence(op_stack[i]);
            /* If not a valid token in math expression - raise an error*/
            if (op_prec == 0 || token_assoc == 3) {
                perror("Not valid token in math expression");
                exit(-1);
            }


            if ((op_prec>token_prec)||(op_prec==token_prec&&token_assoc==0)) {
                
                /* Push the operator to the query */
                query[(*query_size)++] = op_stack[i];
                
                /* And pop it from the operator stack */
                (*op_stack_size)--;

                continue;
            }
            break;
        }
    }

    /* Push the operator token to the operator stack */
    op_stack[(*op_stack_size)++] = token;                    
}

_ASTNodeType _token_type_to_node_type(_token_type type) {
    switch(type)
    {
    case SYMBOL_PLUS:       return OPERATOR_PLUS;
    case SYMBOL_MINUS:      return OPERATOR_MINUS;
    case SYMBOL_MUL:        return OPERATOR_MUL;
    case SYMBOL_DIVIDE:     return OPERATOR_DIV;
    case TYPE_VALUE_NUM:    return CONSTANT;
    case TYPE_VALUE_VAR:    return VARIABLE;
    default:                return ASTNODETYPE_UNDEF;
    }
}

ASTNode* _parse_rpn(_token** query, uint8_t query_size) {
    ASTNode*    root;
    ASTNode*    node;
    
    root = _init_default_ASTNode();
    root->type = _token_type_to_node_type(query[query_size-1]->type);

    if (root->type == ASTNODETYPE_UNDEF) {
        perror("Non valid token");
        exit(-1);
    }

    /*  If the last one is an variable or constant, it indicates that
        there is no mathematical expression */
    if (root->type == VARIABLE || root->type == CONSTANT) {
        root->value = memcpy_str(query[query_size-1]->beg,
                                 query[query_size-1]->length);
    }

    node = root;
    
    for (int i = query_size-2; i >= 0; i-- ) {
        /* Create a new tree sub node if operand */
        if (_is_token_special(query[i])) {
            ASTNode* sub = _init_default_ASTNode();
            sub->type = _token_type_to_node_type(query[i]->type);

            _push_child(node, sub);
            node = &node->children[node->children_num-1];
            
            continue;
        }

        /*  If the binary math operator node is fully filled
            jump to the parent one                          */
        if (node->children_num == 2) {
            node = node->parent;
        }
        
        /*  Create a variable or constant node. Free the memory
            allocated for children - constants and variables
            do not have children nodes.                     */
        ASTNode* value = _init_default_ASTNode();
        free(value->children);
        value->children = NULL;

        value->type = _token_type_to_node_type(query[i]->type);
        value->value = memcpy_str(query[i]->beg, query[i]->length);
        _push_child(node, value);
    }

    return root;
}

ASTNode* _parse(_token* tokens, size_t token_num) {
    /*  `node` holds the current node that is used to store the parsed
        tokens. `token` holds the current token analyzed. `stack`
        is an array of tokens that are stored temporarily until
        an operation is found                                        */
    ASTNode*    root; 
    ASTNode*    node;
    _token*     token;

    _token**    stack;
    uint8_t     stack_size;

    /* For shunting-yard algorithm */
    _token**    op_stack;
    uint8_t     op_stack_size;

    _token**    query;
    uint8_t     query_size;
    

    if (tokens->length == 0) {
        return root;
    }
    root            = _init_default_ASTNode(root);
    node            = root;
    token           = tokens;
    
    stack           = malloc(sizeof(_token) * 256);
    stack_size      = 0;

    op_stack        = malloc(sizeof(_token) * 256);
    op_stack_size   = 0;

    query           = malloc(sizeof(_token) * 256);
    query_size      = 0;


    while (token - tokens < token_num) {
        if (!_is_token_special(token)) {
            stack[stack_size++] = token;
        }

        switch(node->type)
        {
        case ASSIGNMENT:
            /*  In an assignment `ASTNode` it's suitable to have an
                mathematical expression to parse                            */

            /*  Algorithm that converts a infix math expression to postfix 
                aka reverse-polish-notation                                 */
            _shunting_yard_alg(token, query, &query_size, op_stack, &op_stack_size,
                                SYMBOL_SEMICOLON);

            if (token->type == SYMBOL_SEMICOLON) {
                ASTNode* expr = _parse_rpn(query, query_size);
                _push_child(node, expr);
            }
            break;
        case OPERATOR_PLUS:
            break;
        case WHILE_BRANCH:  break;
        case IF_BRANCH:     break;
        case FOR_BRANCH:    break;
        case BRANCH:

            /*"empty" the stack if semicolon detected*/
            if (token->type == SYMBOL_SEMICOLON) {
                stack_size = 0;
                break;
            }            

            switch(token->type)
            {
            case SYMBOL_EQUAL:
                ASTNode* assign = _node_from_token(token, stack, &stack_size);                

                _push_child(node, assign);
                node = &node->children[node->children_num-1];
                stack_size = 0;
                break;
            }

            break;
        default: break;
        }

        token++;
    }

    free(stack);
    return root;
}