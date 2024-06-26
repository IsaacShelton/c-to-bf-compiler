
#ifndef _PECK_AST_TYPEDEF_H
#define _PECK_AST_TYPEDEF_H

#include "utypes.h"

typedef enum {
    TYPEDEF_STRUCT,
    TYPEDEF_ENUM,
} TypeDefKind;

typedef struct {
    // Kind of typedef
    TypeDefKind kind;

    // Line
    u24 line;

    // Name of typedef
    u32 name;

    // Beginning of typedef's fields (stored as statements)
    u32 begin;

    // Number of fields
    u32 num_fields;

    // Computed size
    u32 computed_size;
} TypeDef;

typedef struct {
    u32 name;
    u32 rewritten_type;
} TypeAlias;

#endif // _PECK_AST_TYPEDEF_H

