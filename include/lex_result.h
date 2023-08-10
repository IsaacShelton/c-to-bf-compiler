
#ifndef _PECK_LEX_RESULT_H
#define _PECK_LEX_RESULT_H

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

#endif // _PECK_LEX_RESULT_H

