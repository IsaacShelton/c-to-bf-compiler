
#ifndef _PECK_STACK_DRIVER_H
#define _PECK_STACK_DRIVER_H

#include "utypes.h"
#include "emit_context.h"

u0 emit_stack_driver_pre(u32 entry_basicblock_id);
u0 emit_stack_driver_post();
u32 emit_stack_driver_push_all();
u0 emit_stack_driver_pop_all();

u32 emit_recursive_functions();
u0 emit_stack_pointer();
u0 emit_set_stack_pointer();
u0 emit_stack_push_n(u32 num_cells);
u0 emit_stack_pop_n(u32 num_cells);
u0 emit_start_basicblock(u32 basicblock_id);
u0 emit_end_basicblock();
u32 basicblock_id_for_function(u32 function_id);

u0 emit_start_basicblock_landing(u32 basicblock_id, u32 num_cells_to_pop);
u32 emit_jump(u32 target_basicblock_id);
u32 emit_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells);
u32 emit_end_basicblock_jump(u32 target_basicblock_id);
u0 emit_end_basicblock_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells);
u32 emit_end_basicblock_jump_conditional(u32 then_basicblock_id, u32 else_basicblock_id);
u0 emit_end_basicblock_jump_to(JumpContext context);

#endif // _PACK_STACK_DRIVER_H

