
#ifndef _PECK_EMIT_H
#define _PECK_EMIT_H

#include "utypes.h"
#include "emit_context.h"

u0 dupe_cell();
u0 emit_print_aux_cstr(u32 string);
u0 set_cell_to_value(u8 new_value, u8 old_value);
u0 copy_cell_static(u32 start_index);
u0 copy_cells_static(u32 start_index, u32 size);
u0 copy_cell_dynamic_u8(u32 start_index);
u0 copy_cells_dynamic_u8(u32 start_index, u32 size);
u0 move_cell_static(u32 destination_index);
u0 move_cells_static(u32 destination_index, u32 size, u1 destructive);
u0 move_cell_dynamic_u8(u32 destination_start_index);
u0 move_cells_dynamic_u8(u32 destination_start_index, u32 size);
u0 print_cells_static(u32 start_index, u32 max_length);

u0 emit_printu8();
u0 emit_print_array_value(u32 max_length);
u0 emit_additive_u8(u1 is_plus);
u0 emit_multiply_u8();
u0 emit_divmod_u8();
u0 emit_divide_u8();
u0 emit_mod_u8();
u0 emit_lshift_u8();
u0 emit_rshift_u8();
u0 emit_lt_u8();
u0 emit_gt_u8();

#endif // _PECK_EMIT_H

