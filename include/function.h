
#ifndef _BRAINLOVE_AST_FUNCTION_H
#define _BRAINLOVE_AST_FUNCTION_H

#include "type.h"

typedef struct {
    unsigned int name;
    unsigned char arity;
    Type args[16];
    Type return_type;
    unsigned int begin;
    unsigned int num_stmts;
} Function;

#endif // _BRAINLOVE_AST_FUNCTION_H

