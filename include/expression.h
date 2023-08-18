
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
    EXPRESSION_INDEX,  /* { a, b } */
};
typedef u8 ExpressionKind;

typedef struct {
    ExpressionKind kind;
    u24 line;
    u32 ops; // Start index in `operands` array (except if single operand, in which case ops itself is the value)
} Expression;

u0 expression_print_operator(ExpressionKind kind);
u0 expression_print_operation_name(ExpressionKind kind);
ExpressionKind expression_kind_from_token_kind(TokenKind kind);

#endif // _PECK_AST_EXPRESSION_H

