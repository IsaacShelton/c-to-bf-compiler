
#ifndef _PECK_AST_EXPRESSION_H
#define _PECK_AST_EXPRESSION_H

#include "utypes.h"
#include "token.h"

enum {
    EXPRESSION_NONE,
    EXPRESSION_RETURN, /* { expression } */
    EXPRESSION_DECLARE, /* { type, name } */
    EXPRESSION_PRINT_LITERAL, /* { string } */
    EXPRESSION_PRINT_ARRAY, /* { expression } */
    EXPRESSION_CALL, /* { name, arity, arg1, arg2, ..., argN } */
    EXPRESSION_IMPLEMENT_PUT,
    EXPRESSION_IMPLEMENT_PRINTU1,
    EXPRESSION_IMPLEMENT_PRINTU8,
    EXPRESSION_U1, /* { value } */
    EXPRESSION_U8, /* { value } */
    EXPRESSION_INT, /* { value } */
    EXPRESSION_VARIABLE, /* { name } */
    EXPRESSION_CAST, /* { type, expression } */
    EXPRESSION_ASSIGN, /* { a, b } */
    EXPRESSION_ADD, /* { a, b } */
    EXPRESSION_SUBTRACT, /* { a, b } */
    EXPRESSION_MULTIPLY, /* { a, b } */
    EXPRESSION_DIVIDE, /* { a, b } */
    EXPRESSION_MOD, /* { a, b } */
    EXPRESSION_EQUALS, /* { a, b } */
    EXPRESSION_NOT_EQUALS, /* { a, b } */
    EXPRESSION_LESS_THAN, /* { a, b } */
    EXPRESSION_GREATER_THAN, /* { a, b } */
    EXPRESSION_LESS_THAN_OR_EQUAL, /* { a, b } */
    EXPRESSION_GREATER_THAN_OR_EQUAL, /* { a, b } */
    EXPRESSION_LSHIFT, /* { a, b } */
    EXPRESSION_RSHIFT, /* { a, b } */
    EXPRESSION_AND, /* { a, b } */
    EXPRESSION_OR, /* { a, b } */
    EXPRESSION_BIT_AND, /* { a, b } */
    EXPRESSION_BIT_OR, /* { a, b } */
    EXPRESSION_BIT_XOR, /* { a, b } */
    EXPRESSION_NEGATE, /* { value } */
    EXPRESSION_NOT, /* { value } */
    EXPRESSION_BIT_COMPLEMENT, /* { value } */
    EXPRESSION_INDEX,  /* { a, b } */
    EXPRESSION_PRE_INCREMENT, /* { a } */
    EXPRESSION_PRE_DECREMENT, /* { a } */
    EXPRESSION_POST_INCREMENT, /* { a } */
    EXPRESSION_POST_DECREMENT, /* { a } */
    EXPRESSION_TERNARY, /* { condition, when_true, when_false } */
    EXPRESSION_IF, /* { condition, num_statements } */
    EXPRESSION_IF_ELSE, /* { condition, num_statements, num_else_statements } */
};
typedef u8 ExpressionKind;

typedef struct {
    ExpressionKind kind;
    u24 line;
    u32 ops; // Start index in `operands` array (except if single operand, in which case ops itself is the value)
} Expression;

u0 expression_print_operator(ExpressionKind kind);
u0 expression_print_operation_name(ExpressionKind kind);
ExpressionKind expression_kind_unary_prefix_from_token_kind(TokenKind kind);
ExpressionKind expression_kind_unary_postfix_from_token_kind(TokenKind kind);
ExpressionKind expression_kind_binary_from_token_kind(TokenKind kind);

#endif // _PECK_AST_EXPRESSION_H

