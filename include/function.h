
#ifndef _SMOOCH_AST_FUNCTION_H
#define _SMOOCH_AST_FUNCTION_H

#include "type.h"
#include "utypes.h"

typedef struct {
    // Name of function
    u32 name;
    
    // Number of parameters this function takes
    // NOTE: Exact parameters can be determined by looking at the first `arity` statements (which will be declaration statements)
    u8 arity;

    // Return type of function
    u32 return_type;

    // Beginning of function's statements
    u32 begin;

    // Number of statements in function
    u32 num_stmts;

    // Whether function is recursive
    u1 is_recursive;
} Function;

#endif // _SMOOCH_AST_FUNCTION_H

