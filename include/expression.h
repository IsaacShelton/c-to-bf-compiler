
#ifndef _BRAINLOVE_AST_EXPRESSION_H
#define _BRAINLOVE_AST_EXPRESSION_H

enum {
    EXPRESSION_RETURN, /* { expression } */
    EXPRESSION_DECLARE, /* { type, name } */
    EXPRESSION_PRINT, /* { string } */
    EXPRESSION_CALL, /* { name, arity, arg1, arg2, ..., argN } */
};
typedef unsigned char ExpressionKind;

typedef struct {
    ExpressionKind kind;
    unsigned int ops; // Start index in `operands` array (except if single operand, in which case ops itself is the value)
} Expression;

#endif // _BRAINLOVE_AST_EXPRESSION_H

