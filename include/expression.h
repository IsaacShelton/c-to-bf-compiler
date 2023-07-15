
#ifndef _BRAINLOVE_AST_EXPRESSION_H
#define _BRAINLOVE_AST_EXPRESSION_H

enum {
    EXPRESSION_RETURN, /* { expression } */
    EXPRESSION_DECLARE, /* { type, name } */
};
typedef unsigned char ExpressionKind;

typedef struct {
    ExpressionKind kind;
    unsigned int ops; // Start index in `operands` array
} Expression;

#endif // _BRAINLOVE_AST_EXPRESSION_H

