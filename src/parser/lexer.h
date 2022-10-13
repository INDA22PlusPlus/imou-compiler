#pragma once

#include <stdint.h>
#include <stdbool.h>

#define __TOKEN_REALLOC_SIZE 1024


/* Note: Not everything is going to be implemented */
typedef enum {
    UNDEF,

    /* MAIN TYPES */
    TYPE_U8,    TYPE_I8,
    TYPE_U16,   TYPE_I16,
    TYPE_U32,   TYPE_I32,
    TYPE_U64,   TYPE_I64,
    TYPE_F32,   TYPE_F64,
    TYPE_CHAR,  
    
    TYPE_VALUE_VAR,
    TYPE_VALUE_NUM,

    /* MAIN KEYWORDS */
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_ELSEIF,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_CONTINUE,
    KEYWORD_BREAK,
    KEYWORD_NEW,
    KEYWORD_DELETE,
    KEYWORD_RETURN,

    /* MAIN SYMBOLS */
    SYMBOL_SEMICOLON,       /* ; */
    SYMBOL_OBRACE,          /* { */
    SYMBOL_CBRACE,          /* } */
    SYMBOL_OPARENTHESIS,    /* ( */
    SYMBOL_CPARENTHESIS,    /* ) */
    SYMBOL_OTRIANGLE,       /* < */
    SYMBOL_CTRIANGLE,       /* > */
    SYMBOL_OSQBRACE,        /* [ */
    SYMBOL_CSQBRACE,        /* ] */
    SYMBOL_EQUAL,           /* = */
    SYMBOL_POINTER,         /* ^ */
    SYMBOL_PLUS,            /* + */
    SYMBOL_MINUS,           /* - */
    SYMBOL_DIVIDE,          /* / */
    SYMBOL_MUL,             /* * */
    SYMBOL_NOT,             /* ! */

} _token_type;

/* Lookup table for special chars to enum */
extern const _token_type _LOOKUP_SPECIALCHAR[256];

typedef struct {
    _token_type type;
    
    void* beg;
    uint8_t length;

} _token;

/* Dummy initializer function that sets `.type=UNDEF`, `.length=0`              */
void _default_token(_token* token, char* beg);


/*  Tokenizes the input string based on the given special characters, and keywords.
    Returns a pre-allocated list of all parsed tokens and sets the number of tokens
    parsed into `size_t* token_num`. The program has the responsibility of handling
    the memory of `char* src`, `char* special_chars`, and `char** keywords`.
    All strings should be null-terminated for the tokenizer to work.
    
    Note that strings in `keywords` must be ordered in the way defined below, to
    match the correct `_token_types` in `_lookup_keyword` function:
    keywords[0] -> if
    keywords[1] -> else
    keywords[2] -> for
    keywords[3] -> while
    keywords[4] -> break
    keywords[5] -> continue
    keywords[6] -> I32
                                                                                 */
_token* _tokenize(char* src, char* specialchars, char* whitespace,
                char** keywords, size_t* token_num);

/*  Tries to match the source string with a registred keyword. If unable to
    match to a keyword, either returns `TYPE_VALUE_VAR` or `TYPE_VALUE_NUM`
    based if the parsed string only contains numerical characters or not  */
_token_type _lookup_token_type(char* src, uint8_t src_len, char** keywords);

/*  Returns if the given token is special or not. Used by the parser to
    indicate whether or not a new AST node branch should be created or not*/
bool _is_token_special(_token* token);

/* Checks if the string only contains numerical values */
bool _is_numerical_str(char* src, uint8_t src_len);

/* Checks if the input char is a lexical character; A-Z || a-z */
bool _is_lexical(char c);

/* Checks if the input char is a ascii numerical character; 0-9*/
bool _is_numerical(char c);

/*  Used to identify the math precendence of tokens
    for parsing mathematical expressions         */
uint8_t _token_precedence(_token* token);

/* Math operator associativity. 0 = Left; 1 = Right */
uint8_t _token_associativity(_token* token);
