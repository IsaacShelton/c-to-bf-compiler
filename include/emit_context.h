
#ifndef _PECK_EMIT_CONTEXT_H
#define _PECK_EMIT_CONTEXT_H

#include "utypes.h"

typedef struct {
    u32 basicblock_id;
    u32 num_cells_input;
} JumpContext;

typedef struct {
    u32 num_cases;
    u1 has_default_case;
    u32 first_case_body_basicblock_id;
    u32 first_case_test_basicblock_id;
    u32 default_basicblock_id;
    u32 continuation_basicblock_id;
    u32 pushed;
    u32 next_case;
} SwitchContext;

typedef struct {
    u32 function;
    u32 function_cell_index;
    u32 current_cell_index;
    u32 current_statement;
    u1 in_recursive_function;
    u1 can_function_early_return;
    u32 incomplete_cell;
    u32 didnt_break_cell;
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
    u1 in_basicblock;
    u1 can_break;
    u1 can_continue;
    JumpContext break_basicblock_context;
    JumpContext continue_basicblock_context;
    SwitchContext switch_basicblock_context;
    u32 main_function_index;
} EmitSettings;

extern EmitContext emit_context;
extern EmitSettings emit_settings;

#endif // _PECK_EMIT_CONTEXT_H

