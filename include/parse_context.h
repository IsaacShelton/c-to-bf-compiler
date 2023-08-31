
#ifndef _PECK_PARSE_CONTEXT_H
#define _PECK_PARSE_CONTEXT_H

#include "utypes.h"
#include "token.h"

extern u32 parse_i;
extern u1 had_parse_error;
extern u1 parse_trailing_semicolon;

u0 instead_got();
u0 stop_parsing();
u1 is_token(TokenKind kind);
u32 current_line();
u24 current_line_packed();
u32 eat_word();
u32 eat_int();
u32 eat_string();
u1 eat_token(TokenKind kind);

#endif // _PECK_PARSE_CONTEXT_H

