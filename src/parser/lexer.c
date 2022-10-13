#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"


const _token_type _LOOKUP_SPECIALCHAR[256] = {
    UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,
    UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,
    UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,  UNDEF,
    UNDEF, UNDEF, UNDEF, SYMBOL_NOT, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,  UNDEF,
    SYMBOL_OPARENTHESIS, SYMBOL_CPARENTHESIS, SYMBOL_MUL, SYMBOL_PLUS, UNDEF,
    SYMBOL_MINUS, UNDEF, SYMBOL_DIVIDE, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, SYMBOL_SEMICOLON, SYMBOL_OTRIANGLE,
    SYMBOL_EQUAL, SYMBOL_CTRIANGLE, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    SYMBOL_OSQBRACE, UNDEF, SYMBOL_CSQBRACE, SYMBOL_POINTER, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, SYMBOL_OBRACE, UNDEF, SYMBOL_CBRACE, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
    UNDEF, UNDEF, UNDEF, UNDEF, UNDEF, UNDEF,
};

/* Dummy initializer to clean up the code */
void _default_token(_token* token, char* beg) {
    token->beg = beg;
    token->length = 0;
    token->type = UNDEF;
}

/* Main tokenizer function, check header file for more information */
_token* _tokenize(char* src, char* specialchars, char* whitespace,
                char** keywords, size_t* token_num) {

    char        *c;
    _token      *tokens;
    _token      *last;
    uint16_t    chunk;



    c = src;
    tokens = malloc(sizeof(_token) * __TOKEN_REALLOC_SIZE);
    /*  Pointer to the last VALID element in the token array that is being
        incremented */
    last = tokens;
    /*  Multiple of the realloc chunk size. If the token array needs to be
        expanded, `chunk` is incremented and the new array size becomes
        `chunk * __TOKEN_REALLOC_SIZE`                                  */
    chunk = 1;

    /* Init the first token with default values */
    _default_token(tokens, c);

    while (*c != '\0') {
        
        /* 'Expand' the token array if going out of bounds*/
        if (last - tokens > __TOKEN_REALLOC_SIZE * chunk) {
            /* Store the size temporarily */
            u_int32_t tokens_size = last - tokens;
            
            /* Increment the chunk multiplier */
            chunk++;

            tokens = realloc(tokens, sizeof(_token) * __TOKEN_REALLOC_SIZE * chunk);
            /* Rewrite the pointer to the last element in the array */
            last = tokens + tokens_size;
        }

        /* If the character is a whitespace, '\t', '\n', etc - just ignore it */
        if (strchr(whitespace, *c)) { last->beg++; c++; continue; }

        if (strchr(specialchars, *c)) {
            _token_type type = _LOOKUP_SPECIALCHAR[(uint8_t)(*c)];

            /* Do error if `type = UNDEF` */
            if (type == UNDEF) {
                perror("Non valid character detected");
                abort();
            }

            last->beg = c;
            last->length = 1;
            last->type = type;

            c++;
            /* Initialize the next one  */
            last++;
            _default_token(last, c);
            continue;
        }

        if (!_is_lexical(*c) && !_is_numerical(*c)) {
            /* Do error */
            perror("Non valid character detected");
            abort();
        }

        last->length++;
        c++;

        /*  If the next char is not numerical or lexical, it indicates the end for
            parsing the value or keyword.                                       */
        if (!_is_lexical(*c) && !_is_numerical(*c)) {
            /*  End of value parsing if the next char is not numerical or lexical.
                Check if the parsed value is a registred keyword, or just a simple
                variable/function name or number                                */

            _token_type type = _lookup_token_type(last->beg, last->length, keywords);
            last->type = type;

            /* Initialize the next one  */
            last++;
            _default_token(last, c);
            continue;
        }
    }

    *token_num = last - tokens;
    /* Return the token array with the correct size */
    return realloc(tokens, sizeof(_token) * (last-tokens));
}

bool _is_token_special(_token* token) {
    switch(token->type)
    {
    case KEYWORD_IF:        return true;
    case KEYWORD_ELSE:      return true;
    case KEYWORD_ELSEIF:    return true;
    case KEYWORD_WHILE:     return true;
    case KEYWORD_FOR:       return true;
    
    /* Should reset the stack in `_parse` */
    case SYMBOL_SEMICOLON:  return true;
    case SYMBOL_EQUAL:      return true;
    case SYMBOL_PLUS:       return true;
    case SYMBOL_MINUS:      return true;
    case SYMBOL_DIVIDE:     return true;
    case SYMBOL_MUL:        return true;

    default:                return false;
    }
}

bool _is_lexical(char c) {
    uint8_t t = (uint8_t)c;
    return (t >= 0x41 && t <= 0x5A) || (t >= 0x61 && t <= 0x7A);
}

bool _is_numerical(char c) {
    uint8_t t = (uint8_t)c;
    return t >= 0x30 && t <= 0x39; 
}

/* Checks if all numbers in the string are numerical or not */
bool _is_numerical_str(char* src, uint8_t src_len) {
    for (int i = 0; i < src_len; i++) {
        if (!_is_numerical(src[i])) { return false; }
    }
    return true;
}

/*  Matches the target parsed token value to a registered keyword. If not
    a keyword, returns either a type `TYPE_VALUE_VAR` or `TYPE_VALUE_NUM`   */
_token_type _lookup_token_type(char* src, uint8_t src_len, char** keywords) {
    int matched_index = -1;
    /*  Loop through all allowed keywords and register at what index the
        keyword did match with the source                               */
    for (int i = 0; i < 7; i++) {
        size_t keyword_len = strlen(keywords[i]);

        /* We do memory safe operations in this code */
        if((size_t)src_len != keyword_len) { continue; }

        /*`memcmp` is used instead of `strcmp` because `src` isn't null-terminated*/
        if (memcmp(src, keywords[i], keyword_len) == 0) {
            matched_index = i;
            break;
        }
    }

    switch (matched_index)
    {
    case 0:     return KEYWORD_IF;
    case 1:     return KEYWORD_ELSE;
    case 2:     return KEYWORD_FOR;
    case 3:     return KEYWORD_WHILE;
    case 4:     return KEYWORD_BREAK;
    case 5:     return KEYWORD_CONTINUE;
    case 6:     return TYPE_I32;

    default:
        /*  If the parsed value contains only numerical values,
            return the token type as if it is an numerical value */
        if (_is_numerical_str(src, src_len)) {
            return TYPE_VALUE_NUM;
        }

        /* ...else it is just a variable/function name */
        return TYPE_VALUE_VAR;
    }
}

/*  Used to identify the math precedence of tokens
    for parsing mathematical expressions         */
uint8_t _token_precedence(_token* token) {
    switch(token->type)
    {
    case SYMBOL_PLUS:   return 1;
    case SYMBOL_MINUS:  return 1;
    case SYMBOL_MUL:    return 2;
    case SYMBOL_DIVIDE: return 2;

    default:            return 0;
    }
}

uint8_t _token_associativity(_token* token) {
    switch(token->type)
    {
    case SYMBOL_PLUS:   return 0;
    case SYMBOL_MINUS:  return 0;
    case SYMBOL_MUL:    return 0;
    case SYMBOL_DIVIDE: return 0;
    
    default:            return 3;
    }
}