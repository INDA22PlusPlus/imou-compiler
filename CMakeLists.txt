cmake_minimum_required(VERSION 3.0.0)

project(compiler
    VERSION 1.0
    DESCRIPTION "A compiler for KTH assignment"
    LANGUAGES C)

add_executable(compiler
    src/main.c
    src/compiler/compiler.c
    src/parser/lexer.c
    src/parser/parser.c)

target_compile_options(compiler PRIVATE -Isrc/ -Wall -Wextra -g)