
#include <stdio.h>
#include "../include/config.h"
#include "../include/storage.h"
#include "../include/type.h"
#include "../include/function.h"
#include "../include/expression.h"

char code_buffer[CODE_BUFFER_CAPACITY];
unsigned int code_buffer_length = 0;

Token tokens[TOKENS_CAPACITY];
unsigned int num_tokens = 0;

char aux[AUX_CAPACITY];
unsigned int num_aux = 0;

Type ast_types[TYPES_CAPACITY];
unsigned int num_ast_types = 0;

unsigned int dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
unsigned int num_dimensions = 1; // First is reserved for all zeros

Function functions[FUNCTIONS_CAPACITY];
unsigned int num_functions = 0;

Expression expressions[EXPRESSIONS_CAPACITY];
unsigned int num_expressions = 0;

unsigned int operands[OPERANDS_CAPACITY];
unsigned int num_operands = 0;

void print_aux_cstr(int index){
    for(int i = index; aux[i]; i++){
        putchar(aux[i]);
    }
}

