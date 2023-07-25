
#ifndef _SMOOCH_AST_EXPRESSION_H
#define _SMOOCH_AST_EXPRESSION_H

#include "utypes.h"

enum {
    EXPRESSION_NONE,
    EXPRESSION_RETURN, /* { expression } */
    EXPRESSION_DECLARE, /* { type, name } */
    EXPRESSION_PRINT, /* { string } */
    EXPRESSION_CALL, /* { name, arity, arg1, arg2, ..., argN } */
    EXPRESSION_IMPLEMENT_PUT,
    EXPRESSION_U8, /* { value } */
    EXPRESSION_INT, /* { value } */
};
typedef u8 ExpressionKind;

typedef struct {
    ExpressionKind kind;
    u32 ops; // Start index in `operands` array (except if single operand, in which case ops itself is the value)
} Expression;

#endif // _SMOOCH_AST_EXPRESSION_H

