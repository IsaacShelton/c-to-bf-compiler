
#ifndef _PECK_EXPRESSION_EMIT_H
#define _PECK_EXPRESSION_EMIT_H

#include "expression.h"

typedef struct {
    u32 tape_location;
    u8 offset_size;
    u1 on_stack;
    u32 type; // If >= TYPES_CAPACITY, then error occurred
} Destination;

u32 expression_emit(Expression expression);
Destination expression_get_destination(Expression expression, u32 tape_anchor);
u32 read_destination(Destination destination, u24 line_on_error);
u32 write_destination(u32 new_value_type, Destination destination, u24 line_on_error);
u32 write_destination_unsafe(u32 new_value_type, Destination destination, u24 line_on_error);
ErrorCode grow_type(u32 from_type_index, u32 to_type_index, u8 offset_size, u24 line_on_error);

#endif // _PECK_EXPRESSION_EMIT_H

