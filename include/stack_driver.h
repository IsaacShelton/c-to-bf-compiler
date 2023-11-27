
#ifndef _PECK_STACK_DRIVER_H
#define _PECK_STACK_DRIVER_H

void emit_stack_driver_pre();
void emit_stack_driver_post();
void emit_stack_driver_push_all();
void emit_stack_driver_pop_all();

void emit_stack_push_n(u32 num_cells);
void emit_start_basicblock(u32 basicblock_id);
void emit_end_basicblock();

#endif // _PACK_STACK_DRIVER_H

