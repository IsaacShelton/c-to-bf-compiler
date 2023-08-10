
#ifndef _PECK_EMIT_CONTEXT_H
#define _PECK_EMIT_CONTEXT_H

#include "utypes.h"

typedef struct {
    u32 function_cell_index;
    u32 current_cell_index;
    u32 function_begin_statement;
    u32 current_statement;
    u1 in_recursive_function;
} EmitContext;

EmitContext emit_context;

#endif // _PECK_EMIT_CONTEXT_H

