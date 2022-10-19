#include <stdio.h>
#include <stdlib.h>

#include "compiler/compiler.h"
#include "parser/lexer.h"
#include "parser/parser.h"


char* __read_file(const char* file) {
    FILE    *in;
    size_t  file_size;
    char    *file_buf;

    in = fopen(file, "r");
    if (!in) { perror("Unable to read file"); abort(); }

    fseek(in, 0, SEEK_END);
    file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    file_buf = malloc(file_size+1);
    file_buf[file_size] = '\0';
    fread(file_buf, 1, file_size, in);
    fclose(in);

    return file_buf;
}

int main() {
    
    /* Ordered as they should */
    char* keywords[] = {
        "if",
        "else",
        "elif",
        "for",
        "while",
        "break",
        "continue",
        "i32"
    };

    /* null is not a special char, it just indicates the end of the list*/
    char specialchars[] = {';', '{', '}', '(', ')', '<', '>', '[', ']', '=',
                            '^', '+', '-', '/', '*', ',', '!', '\0'};
    char whitespaces[] = {'\t', ' ', '\n', '\0'};

    size_t token_num = 0;
    
    char* buf = __read_file("examples/lang-files/test.txt");

    _token* tokens = _tokenize(buf, &specialchars[0], &whitespaces[0],
                    &keywords[0], &token_num);

    ASTNode* ast = _parse(tokens, token_num);
    //_stdout_json_serialize_ASTNode(ast);

    _compile(ast, NULL);

    free(tokens);
    free(buf);
    return 0;
}