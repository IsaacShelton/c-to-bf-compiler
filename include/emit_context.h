
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
    u32 incomplete_cell;
    u1 can_break;
    u32 didnt_break_cell;
    u1 can_continue;
    u32 didnt_continue_cell;
    u32 switch_value_type;
    u32 switch_value_type_cached_size;
    u32 switch_start_cell_index;
    u32 fell_through_cell;
} EmitContext;

typedef struct {
    u1 enable_stack;
    u32 stack_pointer;
    u32 stack_begin;
    u32 stack_driver_position;
    u32 next_basicblock_id;
    u1 stack_overflow_checks;
    u32 stack_overflow_message;
    u32 stack_size;
} EmitSettings;

extern EmitContext emit_context;
extern EmitSettings emit_settings;

#endif // _PECK_EMIT_CONTEXT_H

