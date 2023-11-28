
#ifndef _PECK_STACK_DRIVER_H
#define _PECK_STACK_DRIVER_H

#include "utypes.h"

void emit_stack_driver_pre(u32 entry_basicblock_id);
void emit_stack_driver_post();
u32 emit_stack_driver_push_all();
void emit_stack_driver_pop_all();

u32 emit_recursive_functions();
void emit_stack_pointer();
void emit_set_stack_pointer();
void emit_stack_push_n(u32 num_cells);
void emit_stack_pop_n(u32 num_cells);
void emit_start_basicblock(u32 basicblock_id);
void emit_end_basicblock();
u32 basicblock_id_for_function(u32 function_id);

void emit_start_basicblock_landing(u32 basicblock_id, u32 num_cells_to_pop);
u32 emit_jump(u32 target_basicblock_id);
u32 emit_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells);
u32 emit_end_basicblock_jump(u32 target_basicblock_id);
void emit_end_basicblock_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells);
u32 emit_end_basicblock_jump_conditional(u32 then_basicblock_id, u32 else_basicblock_id);

#endif // _PACK_STACK_DRIVER_H

