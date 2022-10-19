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

const char* _node_type_to_str(_ASTNodeType type) {
    switch (type)
    {
    case FUNCTION_CALL:         return "FUNCTION CALL";
    case FUNCTION_DEF:          return "FUNCTION DEFINITION";
    case VARIABLE:              return "VARIABLE";
    case CONSTANT:              return "CONSTANT";
    case ASSIGNMENT:            return "ASSIGNMENT";
    case EXPRESSION:            return "EXPRESSION";
    case COMPARE:               return "COMPARISON";
    case BRANCH:                return "BRANCH";
    case IF_BRANCH:             return "IF";
    case ELSE_BRANCH:           return "ELSE";
    case ELSEIF_BRANCH:         return "ELSE IF";
    case WHILE_BRANCH:          return "WHILE";
    case FOR_BRANCH:            return "FOR";
    case OPERATOR_PLUS:         return "+";
    case OPERATOR_MINUS:        return "-";
    case OPERATOR_DIV:          return "/";
    case OPERATOR_MUL:          return "*";
    default:                    return "UNDEF";
    }
}

static const char* _comparison_to_str(_token* token) {
    switch (token->type)
    {
    case SYMBOL_OTRIANGLE:      return "<";
    case SYMBOL_CTRIANGLE:      return ">";
    case COMPARE_EQUAL:         return "==";
    case COMPARE_NEQUAL:        return "!=";
    default:                    return NULL;
    }
}

static ASTNode* _parent(ASTNode* node, _token* next) {
    ASTNode* parent;

    parent = node->parent;
    switch (parent->type)
    {
    case IF_BRANCH:
        if (parent->children_num < 2) {
            free(node);
            perror("Syntax error: Illegal `if` statement");
            abort();
        }

        if (next->type != KEYWORD_ELSE &&
            next->type != KEYWORD_ELSEIF) {
            parent = parent->parent;
            break;
        }
        return parent;
    case ELSEIF_BRANCH:
        if (parent->children_num < 2) {
            free(node);
            perror("Syntax error: Illegal `elif` statement");
            abort();
        }
        if (next->type != KEYWORD_ELSE &&
            next->type != KEYWORD_ELSEIF) {
            parent = parent->parent->parent;
            break;
        }
        /* Leave the whole if statement if no else or elif */
        return parent->parent;
    case WHILE_BRANCH:
        if (parent->children_num < 2) {
            free(node);
            perror("Syntax error: Illegal `while` statement");
            abort();
        }
        if (parent->children_num == 2) {
            parent = parent->parent;
            break;
        }
        return parent;

    default:    break;
    }

    return parent;
}

/*  Initializes an empty ASTNode with NULL values and `BRANCH` type,
    just to clean up the code                                        */
static ASTNode* _init_default_ASTNode() {
    ASTNode* target = malloc( sizeof(ASTNode) );
    
    target->parent          = NULL;
    target->children        = malloc(sizeof(ASTNode)*__ASTNODE_REALLOC_SIZE);
    target->children_num    = 0;
    target->type            = BRANCH;
    target->value           = NULL;
    target->options         = NULL;

    return target;
}

/* Pushes the given child to the parent. Handles capacity management */
static void _push_child(ASTNode* parent, ASTNode* child) {
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

/*  Tries to construct a variable `ASTNode` from the given
    stack                                                            */
static void _build_var_from_stack(ASTNode* node, _token** stack, uint8_t stack_size) {
    node->type          = VARIABLE;
    node->capacity      = 0;
    node->children      = NULL;
    node->children_num  = 0;

    /*  Simple check if the format is valid.
        `stack_size` == 1: already defined variable;
        `stack_size` == 4: new variable with given type*/
        
    /* When memory overflow without error is actually a good thing*/
    /* 1+1 and 4+1, the assignment operator is in stack and it counts too*/
    if ((stack_size != 2 || stack_size != 5) && 
        (stack[0]->type != TYPE_VALUE_VAR)) {
    
        free(stack);
        free(node);
        perror("Not valid variable identifier");
        abort();
    }

    node->value = memcpy_str(stack[0]->beg, stack[0]->length);
    
    if (stack_size == 2) {return;}

    if (stack[1]->type != SYMBOL_OTRIANGLE ||
        stack[3]->type != SYMBOL_CTRIANGLE) {

        free(stack);
        free(node);
        perror("Not valid variable identifier");
        abort();
    }

    node->options = malloc(sizeof(char*) * 1);
    node->options[0] = memcpy_str(stack[2]->beg, stack[2]->length);
}

/* Creates valid ASTNode based on the token type and stack input */
static ASTNode* _node_from_token(_token* token, _token** stack,
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
        if (stack[(*stack_size)-2]->type == TYPE_VALUE_VAR) {
            target->type = FUNCTION_CALL;
            target->value = memcpy_str(stack[*stack_size-2]->beg,
                                        stack[*stack_size-2]->length);
        }
        break;

    case SYMBOL_EQUAL:
        target->type = ASSIGNMENT;
        
        /*  Must include a variable at left side */
        _build_var_from_stack(&target->children[0], stack, *stack_size);
        target->children_num++;

        break;
    case KEYWORD_IF:
        target->type = IF_BRANCH;
        break;
    case KEYWORD_ELSE:
        target->type = ELSE_BRANCH;
        break;
    case KEYWORD_ELSEIF:
        target->type = ELSEIF_BRANCH;
        break;
    case KEYWORD_WHILE:
        target->type = WHILE_BRANCH;
        break;
    case KEYWORD_FOR:
        target->type = FOR_BRANCH;
        break;

    /* This implies that all mathematical expressions that are outside
        assignments, are going to be ignored*/
    default:    return NULL;
    }
    
    *stack_size = 0;
    return target;
}

static _ASTNodeType _token_type_to_node_type(_token_type type) {
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


/* Empties and pops the operator stack into the query */
void _shunting_yard_alg_force_clean(_token** query, uint8_t* query_size,
                                    _token** op_stack, uint8_t* op_stack_size) {

    if (*op_stack_size == 0) { return; }

    for(int i = *op_stack_size-1; i >= 0; i--) {
        query[(*query_size)++] = op_stack[i];
    }
    
    *op_stack_size = 0;
}

/* Shunting-yard algorithm to parse mathematical expressions*/
void _shunting_yard_alg(_token* token, _token** query, uint8_t* query_size,
                        _token** op_stack, uint8_t* op_stack_size,
                         _token_type end_symbol) {

    /* Empty the stack on expression end */
    if (token->type == end_symbol) {
        _shunting_yard_alg_force_clean(query, query_size, 
                                        op_stack, op_stack_size);
        return;
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
                free(query);
                free(op_stack);
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

ASTNode* _parse_rpn(_token** query, uint8_t* query_size) {
    ASTNode*    root;
    ASTNode*    node;
    
    root = _init_default_ASTNode();
    root->type = _token_type_to_node_type(query[(*query_size)-1]->type);

    if (root->type == ASTNODETYPE_UNDEF) {
        free(query);
        perror("Non valid token");
        exit(-1);
    }

    /*  If the last one is an variable or constant, it indicates that
        there is no mathematical expression */
    if (root->type == VARIABLE || root->type == CONSTANT) {
        root->value = memcpy_str(query[(*query_size)-1]->beg,
                                 query[(*query_size)-1]->length);
    }

    node = root;
    
    for (int i = (*query_size)-2; i >= 0; i-- ) {
        /*  If the binary math operator node is fully filled
            jump to the parent one that has only one child */
        if (node->children_num == 2) {
            while (node->children_num == 2) {
                node = _parent(node, query[i+1]);
            }
        }
        /* Create a new tree sub node if operand */
        if (_is_token_special(query[i])) {
            ASTNode* sub = _init_default_ASTNode();
            sub->type = _token_type_to_node_type(query[i]->type);

            _push_child(node, sub);
            node = &node->children[node->children_num-1];
            
            continue;
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

    /* Parsed the query, just "empty" it*/
    *query_size = 0;

    return root;
}

static ASTNode* _parse_comparison(_token** stack, uint8_t stack_size) {
    _token**    query;
    _token**    op_stack;
    
    uint8_t     query_size;
    uint8_t     op_stack_size;

    ASTNode*    node;
    

    query = malloc(sizeof(_token*) * 256);
    op_stack = malloc(sizeof(_token*) * 256);

    query_size = 0;
    op_stack_size = 0;

    node = _init_default_ASTNode();
    node->type = COMPARE;


    if (stack[0]->type != SYMBOL_OPARENTHESIS ||
        stack[stack_size-1]->type != SYMBOL_CPARENTHESIS ||
        stack_size == 2) {
        free(query);
        free(op_stack);
        perror("Syntax error: problem with `while`");
        abort();
    }

    for (int i = 1; i < stack_size - 1; i++ ) {
        if (_token_compare_op(stack[i])) {
            /* Encode the integer type as a char */
            node->value = strdup(_comparison_to_str(stack[i]));

            _shunting_yard_alg_force_clean(query, &query_size,
                                            op_stack, &op_stack_size);
            ASTNode* part1 = _parse_rpn(query, &query_size);
            _push_child(node, part1);
        } else {
            _shunting_yard_alg(stack[i], query, &query_size,
                                op_stack, &op_stack_size, UNDEF);
        }
    }
    _shunting_yard_alg_force_clean(query, &query_size,
                                    op_stack, &op_stack_size);
    ASTNode* part2 = _parse_rpn(query, &query_size);
    _push_child(node, part2);

    free(query);
    free(op_stack);
    return node;
}


/*  MAIN PARSE FUNCTION USED TO CREATE AN AST FROM A LINEAR LIST OF TOKENS */
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
        return NULL;
    }
    root            = _init_default_ASTNode();
    node            = root;
    token           = tokens;
    
    stack           = malloc(sizeof(_token) * 256);
    stack_size      = 0;

    op_stack        = malloc(sizeof(_token) * 256);
    op_stack_size   = 0;

    query           = malloc(sizeof(_token) * 256);
    query_size      = 0;


    while (token - tokens < token_num) {
        stack[stack_size++] = token;

        if (token->type == SYMBOL_CBRACE) {

            node = _parent(node, token+1);
            stack_size = 0;
            token++;
            continue;
        }

        switch(node->type)
        {
        case FUNCTION_CALL:
            ASTNode*        sub;

            switch (token->type)
            {
            case SYMBOL_CPARENTHESIS:
                _shunting_yard_alg_force_clean(query, &query_size, op_stack,
                                                &op_stack_size);
                 sub = _parse_rpn(query, &query_size);
                _push_child(node, sub);

                /* End of function call, just jump to the parent */
                node = _parent(node, token+1);
                break;
            case SYMBOL_COMMA:
                /* Pops the operator stack on comma*/
                _shunting_yard_alg(token, query, &query_size, op_stack, &op_stack_size,
                                    SYMBOL_COMMA);
                sub = _parse_rpn(query, &query_size);
                _push_child(node, sub);
                break;
            default:
                _shunting_yard_alg(token, query, &query_size, op_stack, &op_stack_size,
                    SYMBOL_COMMA);
                break;
            }

            break;
        case ASSIGNMENT:
            /*  In an assignment `ASTNode` it's suitable to have an
                mathematical expression to parse                            */

            /*  Algorithm that converts a infix math expression to postfix 
                aka reverse-polish-notation                                 */
            _shunting_yard_alg(token, query, &query_size, op_stack, &op_stack_size,
                                SYMBOL_SEMICOLON);

            if (token->type == SYMBOL_SEMICOLON) {
                ASTNode* expr = _parse_rpn(query, &query_size);
                _push_child(node, expr);
                node = _parent(node, token+1);
            }
            break;
        case ELSEIF_BRANCH:
            if (node->parent->type != IF_BRANCH) {
                free(tokens);
                free(root);
                perror("Syntax error: `elif` not in `if` block");
                exit(-1);
            }
            /* FALLTHROUGH */
            /*  People need to learn to exploit switch case fallthoughs,
                than hating on them so much                             */
        case IF_BRANCH:
            if (node->children_num > 0 && node->type != ELSEIF_BRANCH) {
                ASTNode* _sub = _node_from_token(token, stack, &stack_size);                
                if (!_sub) { break; }

                /* If the next branch is an else branch,
                    ignore the opening brace            */
                if (_sub->type == ELSE_BRANCH
                    && (token+1)->type == SYMBOL_OBRACE) {
                    token++;
                }

                _push_child(node, _sub);
                node = &node->children[node->children_num-1];
                stack_size = 0;
                break;
            }
            /* FALLTHROUGH */
        case WHILE_BRANCH:
            /* Indicates the end of condition expression */
            if (node->children_num == 0 && token->type == SYMBOL_OBRACE) {
                /* Creates the condition subnode and does error handling */
                ASTNode* compare = _parse_comparison(stack, stack_size-1);
                _push_child(node, compare);

                /*  Create a new branch which is the body of the while
                    loop*/
                ASTNode* branch  = _init_default_ASTNode();
                _push_child(node, branch);
                /* After `push_child`, the `branch` node memory is freed */
                node = &node->children[node->children_num-1];
                
                stack_size = 0;
            }
            break;
        case FOR_BRANCH:    break;
        /* Else branch does not have any compare node operators,
            so it's just another branch. Interpret it as an
            `BRANCH` node.                                  */
        case ELSE_BRANCH:
        case BRANCH:          

            ASTNode* _sub = _node_from_token(token, stack, &stack_size);                
            if (!_sub) { break; }

            _push_child(node, _sub);
            node = &node->children[node->children_num-1];
            stack_size = 0;
            

            break;
        default:    break;
        }


        token++;
    }

    /*"empty" the stack if semicolon detected*/
    if (token->type == SYMBOL_SEMICOLON) {
        stack_size = 0;
    }  

    free(stack);
    return root;
}

void _stdout_json_serialize_ASTNode(ASTNode* root) {
    if (!root) { return; }

    printf("{\"type\": \"%s\"", _node_type_to_str(root->type));
    if (root->value) {
        printf(", \"value\": \"%s\"", root->value);
    }

    if (root->options) {
        printf(", \"options\": \"%s\"", *root->options);
    }
 
    if (root->children_num > 0) {
        printf(", \"children\": [");

        for (int i = 0; i < root->children_num; i++) {
            _stdout_json_serialize_ASTNode(&root->children[i]);
            if (i != root->children_num - 1) {
                printf(", ");
            }
        }
        printf("]");
    }
    printf("}");

}