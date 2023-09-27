
#ifndef _PECK_AST_TYPE_H
#define _PECK_AST_TYPE_H

#include "utypes.h"

typedef struct {
    u32 name;
    u32 dimensions;
} Type;

u0 type_does_not_exist(Type type, u24 line);

#endif // _PECK_AST_TYPE_H

