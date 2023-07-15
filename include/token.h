
#ifndef _BRAINLOVE_TOKEN_H
#define _BRAINLOVE_TOKEN_H

enum {
    TOKEN_NONE,
    TOKEN_DONE,
    TOKEN_ERROR,
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
};

typedef unsigned char TokenKind;

typedef struct {
    TokenKind kind;
    int data;
    int line;
} Token;

#endif // _BRAINLOVE_TOKEN_H

