
#ifndef _BRAINLOVE_AST_EXPRESSION_H
#define _BRAINLOVE_AST_EXPRESSION_H

enum {
    EXPRESSION_RETURN,
};
typedef unsigned char ExpressionKind;

typedef struct {
    ExpressionKind kind;
    unsigned int ops;
} Expression;

#endif // _BRAINLOVE_AST_EXPRESSION_H

