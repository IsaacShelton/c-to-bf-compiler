
#ifndef _PECK_TOKEN_H
#define _PECK_TOKEN_H

#include "utypes.h"

enum {
    TOKEN_NONE,
    TOKEN_DONE,
    TOKEN_ERROR,
    TOKEN_LINE_COMMENT,
    TOKEN_MULTILINE_COMMENT,
    TOKEN_WORD,
    TOKEN_BEGIN,
    TOKEN_END,
    TOKEN_OPEN,
    TOKEN_CLOSE,
    TOKEN_STRING,
    TOKEN_SEMICOLON,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_INT,
    TOKEN_NEXT,
    TOKEN_ASSIGN,
    TOKEN_ADD,
    TOKEN_SUBTRACT,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LESS_THAN_OR_EQUAL,
    TOKEN_GREATER_THAN_OR_EQUAL,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_NOT,
    TOKEN_BIT_COMPLEMENT,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_TERNARY,
    TOKEN_COLON,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_TYPEDEF,
    TOKEN_STRUCT,
    TOKEN_MEMBER,
};

typedef u8 TokenKind;

typedef struct {
    TokenKind kind;
    u24 line;
    u32 data;
} Token;

#endif // _PECK_TOKEN_H

