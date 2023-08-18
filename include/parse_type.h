
#ifndef _PECK_PARSE_TYPE_H
#define _PECK_PARSE_TYPE_H

#include "type.h"
#include "token.h"
#include "utypes.h"

Type parse_type();
u1 is_type_followed_by(TokenKind followed_by);

#endif // _PECK_PARSE_TYPE_H

