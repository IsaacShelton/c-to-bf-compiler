
#ifndef _PECK_STACK_DRIVER_H
#define _PECK_STACK_DRIVER_H

#include "utypes.h"

void emit_stack_driver_pre(u32 entry_basicblock_id);
void emit_stack_driver_post();
void emit_stack_driver_push_all();
void emit_stack_driver_pop_all();

void emit_recursive_functions();
void emit_stack_push_n(u32 num_cells);
void emit_stack_pop_n(u32 num_cells);
void emit_start_basicblock(u32 basicblock_id);
void emit_end_basicblock();
u32 basicblock_id_for_function(u32 function_id);

#endif // _PACK_STACK_DRIVER_H

