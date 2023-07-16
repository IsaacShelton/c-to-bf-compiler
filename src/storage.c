
#include <stdio.h>
#include <stdbool.h>
#include "../include/config.h"
#include "../include/storage.h"
#include "../include/type.h"
#include "../include/function.h"
#include "../include/expression.h"
#include "../include/global.h"
#include "../include/type_equals.h"

char code_buffer[CODE_BUFFER_CAPACITY];
unsigned int code_buffer_length = 0;

Token tokens[TOKENS_CAPACITY];
unsigned int num_tokens = 0;

char aux[AUX_CAPACITY];
unsigned int num_aux = 0;

Type types[TYPES_CAPACITY];
unsigned int num_types = 0;

unsigned int dimensions[UNIQUE_DIMENSIONS_CAPACITY][4];
unsigned int num_dimensions = 1; // First is reserved for all zeros

Function functions[FUNCTIONS_CAPACITY];
unsigned int num_functions = 0;

Expression expressions[EXPRESSIONS_CAPACITY];
unsigned int num_expressions = 0;

int statements[STATEMENTS_CAPACITY];
unsigned int num_statements = 0;

unsigned int operands[OPERANDS_CAPACITY];
unsigned int num_operands = 0;

Global globals[GLOBALS_CAPACITY];
unsigned int num_globals;

int add_type(Type type){
    for(int i = 0; i < num_types; i++){
        if(type_equals(type, types[i])){
            return i;
        }
    }

    if(num_types < TYPES_CAPACITY){
        types[num_types] = type;
        return num_types++;
    }

    return TYPES_CAPACITY;
}

int add_expression(Expression expression){
    if(num_expressions < EXPRESSIONS_CAPACITY){
        expressions[num_expressions] = expression;
        return num_expressions++;
    } else {
        return EXPRESSIONS_CAPACITY;
    }
}

int add_statement_from_existing(int expression_index){
    if(num_statements < STATEMENTS_CAPACITY){
        statements[num_statements] = expression_index;
        return num_statements++;
    } else {
        return STATEMENTS_CAPACITY;
    }
}

int add_statement_from_new(Expression expression){
    int expression_index = add_expression(expression);

    if(expression_index < EXPRESSIONS_CAPACITY){
        return add_statement_from_existing(expression_index);
    } else {
        return STATEMENTS_CAPACITY;
    }
}

int add_operands2(int a, int b){
    if(num_operands < OPERANDS_CAPACITY - 2){
        int result = num_operands;
        operands[num_operands] = a;
        operands[num_operands + 1] = b;
        num_operands += 2;
        return result;
    } else {
        printf("Out of memory: Exceeded maximum number of total expression operands\n");
        return OPERANDS_CAPACITY;
    }
}

int add_global(Global global){
    if(num_globals < GLOBALS_CAPACITY){
        globals[num_globals] = global;
        return num_globals++;
    }

    return GLOBALS_CAPACITY;
}

bool aux_cstr_equals(int a, int b){
    for(int i = 0; aux[a + i] == aux[b + i]; i++){
        if(aux[a + i] == '\0'){
            return true;
        }
    }
    return false;
}

bool aux_cstr_equals_print(int a){
    return a < AUX_CAPACITY - 5
        && aux[a    ] == 'p'
        && aux[a + 1] == 'r'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'n'
        && aux[a + 4] == 't'
        && aux[a + 5] == '\0';
}

bool aux_cstr_equals_main(int a){
    return a < AUX_CAPACITY - 4
        && aux[a    ] == 'm'
        && aux[a + 1] == 'a'
        && aux[a + 2] == 'i'
        && aux[a + 3] == 'n'
        && aux[a + 4] == '\0';
}

void print_aux_cstr(int index){
    for(int i = index; aux[i]; i++){
        putchar(aux[i]);
    }
}

