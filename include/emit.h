
#ifndef _SMOOCH_EMIT_H
#define _SMOOCH_EMIT_H

#include "utypes.h"
#include "emit_context.h"

u0 set_cell_to_value(u8 new_value, u8 old_value);
u0 emit_print_aux_cstr(u32 string);
u0 copy_cell_static(u32 start_index);
u0 copy_cells_static(u32 start_index, u32 size);
u0 move_cell_static(u32 destination_index);
u0 move_cells_static(u32 destination_index, u32 size, u1 destructive);

u0 emit_additive_u8(u1 is_plus);

#endif // _SMOOCH_EMIT_H

