
#ifndef _PECK_PARSE_MACRO_H
#define _PECK_PARSE_MACRO_H

#include "type.h"
#include "token.h"
#include "utypes.h"
#include "expression.h"

ErrorCode parse_macro();
Expression perform_compile_time_math(Expression expression);

#endif // _PECK_PARSE_MACRO_H

