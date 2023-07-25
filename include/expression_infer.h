
#ifndef _SMOOCH_EXPRESSION_INFER_H
#define _SMOOCH_EXPRESSION_INFER_H

#include "../include/storage.h"

ExpressionKind expression_get_preferred_int_kind_or_none(u32 expression_index);
u0 expression_infer(u32 expression_index, ExpressionKind preferred_int_kind);

#endif // _SMOOCH_EXPRESSION_INFER_H

