
#ifndef _PECK_EMIT_CONTEXT_H
#define _PECK_EMIT_CONTEXT_H

#include "utypes.h"

typedef struct {
    u32 function;
    u32 function_cell_index;
    u32 current_cell_index;
    u32 current_statement;
    u1 in_recursive_function;
    u1 can_function_early_return;
    u16 num_early_return_closes_needed;
    u32 incomplete_cell;
    u1 can_break;
    u32 didnt_break_cell;
    u16 num_break_check_closes_needed;
} EmitContext;

EmitContext emit_context;

#endif // _PECK_EMIT_CONTEXT_H

