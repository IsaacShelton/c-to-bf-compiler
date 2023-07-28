
#ifndef _SMOOCH_VARIABLE_FIND_H
#define _SMOOCH_VARIABLE_FIND_H

#include "utypes.h"
#include "../include/expression_emit.h"

typedef struct {
    u1 on_stack; // Whether variable lives on the stack
    u32 location; // Absolute location if on tape, or relative location if on stack
} VariableLocation;

typedef struct {
    u32 name; // name index in `aux`
    u32 type; // type index in `types`
    u32 depth; // scope depth, depth of 0 means global, anything else means in a scope
    u32 declaration; // index of declaration, (global index if global otherwise statement index)
    u1 defined; // whether defined
    VariableLocation location;
} Variable;

// Tape layout:
// [global variables] [stack pointer] [stack] [working memory]

Variable variable_find(u32 name);

#endif // _SMOOCH_VARIABLE_FIND_H

