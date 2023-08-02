
#ifndef _SMOOCH_LEX_RESULT_H
#define _SMOOCH_LEX_RESULT_H

#include "utypes.h"
#include "token.h"

typedef struct {
    Token token;
    u32 consumed;
} LexedToken;

typedef struct {
    u1 error;
    u8 new_c;
} LexUnboundedResult;

#endif // _SMOOCH_LEX_RESULT_H

