
#ifndef _BRAINLOVE_AST_FUNCTION_H
#define _BRAINLOVE_AST_FUNCTION_H

#include "type.h"

typedef struct {
    // Name of function
    unsigned int name;
    
    // Number of parameters this function takes
    // NOTE: Exact parameters can be determined by looking at the first `arity` statements (which will be declaration statements)
    unsigned char arity;

    // Return type of function
    int return_type;

    // Beginning of function's statements
    unsigned int begin;

    // Number of statements in function
    unsigned int num_stmts;
} Function;

#endif // _BRAINLOVE_AST_FUNCTION_H

