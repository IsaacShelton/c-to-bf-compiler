
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
    TOKEN_INT,
    TOKEN_NEXT,
    TOKEN_EQUALS,
    TOKEN_ASSIGN,
    TOKEN_ADD,
    TOKEN_SUBTRACT,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
};

typedef u8 TokenKind;

typedef struct {
    TokenKind kind;
    u24 line;
    u32 data;
} Token;

#endif // _PECK_TOKEN_H

