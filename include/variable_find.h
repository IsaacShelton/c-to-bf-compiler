
#ifndef _PECK_VARIABLE_FIND_H
#define _PECK_VARIABLE_FIND_H

#include "utypes.h"
#include "../include/expression_emit.h"

typedef enum {
    VARIABLE_LOCATION_ON_TAPE,
    VARIABLE_LOCATION_ON_STACK,
    VARIABLE_LOCATION_IMMUTABLE,
} VariableLocationKind;

typedef struct {
    VariableLocationKind kind;

    // Absolute location if on tape,
    // Relative location if on stack,
    // Value if immutable
    u32 location;
} VariableLocation;

typedef struct {
    u32 name; // name index in `aux`
    u32 type; // type index in `types`
    u32 depth; // scope depth, depth of 0 means global, anything else means in a scope
    u1 defined; // whether defined
    VariableLocation location;
} Variable;

// Tape layout:
// [global variables] [stack pointer] [stack] [working memory]

Variable variable_find(u32 name);

#endif // _PECK_VARIABLE_FIND_H

