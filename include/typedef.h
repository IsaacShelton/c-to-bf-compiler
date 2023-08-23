
#ifndef _PECK_AST_TYPEDEF_H
#define _PECK_AST_TYPEDEF_H

#include "utypes.h"

typedef enum {
    TYPEDEF_STRUCT,
} TypeDefKind;

typedef struct {
    // Kind of typedef
    TypeDefKind kind;

    // Name of typedef
    u32 name;

    // Beginning of typedef's fields (stored as statements)
    u32 begin;

    // Number of fields
    u32 num_fields;

    // Computed size
    u32 computed_size;
} TypeDef;

#endif // _PECK_AST_TYPEDEF_H

