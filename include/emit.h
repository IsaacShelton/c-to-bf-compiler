
#ifndef _PECK_EMIT_H
#define _PECK_EMIT_H

#include "utypes.h"
#include "emit_context.h"

u0 dupe_cell();
u0 dupe_cell_offset(u32 amount);
u0 dupe_cells(u32 amount);
u0 emit_print_aux_cstr(u32 string);
u0 set_cell_to_value(u8 new_value, u8 old_value);

u0 copy_cell_static(u32 start_index);
u0 copy_cells_static(u32 start_index, u32 size);

u0 copy_cell_dynamic_u8(u32 start_index);
u0 copy_cell_dynamic_u16(u32 start_index);
u0 copy_cell_dynamic_u24(u32 start_index);
u0 copy_cell_dynamic_u32(u32 start_index);

u0 copy_cells_dynamic_u8(u32 start_index, u32 size);
u0 copy_cells_dynamic_u16(u32 start_index, u32 size);
u0 copy_cells_dynamic_u24(u32 start_index, u32 size);
u0 copy_cells_dynamic_u32(u32 start_index, u32 size);

u0 move_cell_static(u32 destination_index);
u0 move_cells_static(u32 destination_index, u32 size, u1 destructive);

u0 move_cell_dynamic_u8(u32 destination_start_index);
u0 move_cell_dynamic_u16(u32 destination_start_index);
u0 move_cell_dynamic_u24(u32 destination_start_index);
u0 move_cell_dynamic_u32(u32 destination_start_index);

u0 move_cells_dynamic_u8(u32 destination_start_index, u32 size);
u0 move_cells_dynamic_u16(u32 destination_index, u32 size);
u0 move_cells_dynamic_u24(u32 destination_index, u32 size);
u0 move_cells_dynamic_u32(u32 destination_index, u32 size);

u0 print_cells_static(u32 start_index, u32 max_length);

u0 emit_u1(u1 value);
u0 emit_u8(u8 value);
u0 emit_u16(u16 value);
u0 emit_u24(u24 value);
u0 emit_u32(u32 value);

u0 emit_printu1();
u0 emit_printu8();
u0 emit_print_array_value(u32 max_length);

u0 emit_additive_u8(u1 is_plus);
u0 emit_additive_u16(u1 is_plus);
u0 emit_additive_u32(u1 is_plus);

u0 emit_multiply_u8();
u0 emit_multiply_u16();
u0 emit_multiply_u32();

u0 emit_divmod_u8();

u0 emit_divide_u8();
u0 emit_divide_u16();
u0 emit_divide_u32();

u0 emit_mod_u8();
u0 emit_mod_u16();
u0 emit_mod_u32();

u0 emit_lshift_u8();
u0 emit_lshift_u16();
u0 emit_lshift_u32();

u0 emit_rshift_u8();
u0 emit_rshift_u16();
u0 emit_rshift_u32();

u0 emit_eq_u8();
u0 emit_eq_u16();
u0 emit_eq_u32();

u0 emit_neq_u8();
u0 emit_neq_u16();
u0 emit_neq_u32();

u0 emit_lt_u8();
u0 emit_lt_u16();
u0 emit_lt_u32();

u0 emit_gt_u8();
u0 emit_gt_u16();
u0 emit_gt_u32();

u0 emit_lte_u8();
u0 emit_lte_u16();
u0 emit_lte_u32();

u0 emit_gte_u8();
u0 emit_gte_u16();
u0 emit_gte_u32();

u0 emit_bit_and_u8();
u0 emit_bit_and_u16();
u0 emit_bit_and_u32();

u0 emit_bit_or_u8();
u0 emit_bit_or_u16();
u0 emit_bit_or_u32();

u0 emit_bit_xor_u8();
u0 emit_bit_xor_u16();
u0 emit_bit_xor_u32();

u0 emit_not_u1();

u0 emit_negate_u8();
u0 emit_negate_u16();
u0 emit_negate_u32();

u0 emit_bit_complement_u8();
u0 emit_bit_complement_u16();
u0 emit_bit_complement_u32();

#endif // _PECK_EMIT_H

