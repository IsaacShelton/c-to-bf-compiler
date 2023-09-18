
#ifndef _PECK_EXPRESSION_GET_TYPE_H
#define _PECK_EXPRESSION_GET_TYPE_H

#include "utypes.h"
#include "type.h"
#include "expression.h"

u32 expression_get_type(Expression expression, u1 print_error);
u32 get_item_type(Type type, u24 line_on_error, u1 show_error_message);
u32 expression_get_type_for_call(Expression expression);
u32 expression_get_type_for_string(Expression expression);
u32 expression_get_type_for_variable(Expression expression, u1 print_error);

#endif // _PECK_EXPRESSION_GET_TYPE_H

