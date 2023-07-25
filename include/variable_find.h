
#ifndef _SMOOCH_VARIABLE_FIND_H
#define _SMOOCH_VARIABLE_FIND_H

#include "utypes.h"

typedef struct {
    u32 name; // name index in `aux`
    u32 type; // type index in `types`
    u32 depth; // scope depth, depth of 0 means global, anything else means in a scope
    u32 declaration; // index of declaration, (global index if global otherwise statement index)
    u1 defined; // whether defined
} Variable;

Variable variable_find(u32 function_beginning, u32 current_statement, u32 name);

#endif // _SMOOCH_VARIABLE_FIND_H

