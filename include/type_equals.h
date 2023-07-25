
#ifndef _SMOOCH_TYPE_EQUALS_H
#define _SMOOCH_TYPE_EQUALS_H

#include "type.h"
#include "utypes.h"

// NOTE: If using registered types (indices inside of global `types` array),
//       You should just check if two indicies are equal.
//       e.g. return_type == u0_type
// Determines if two `Type` values are equal.
u1 type_equals(Type a, Type b);

#endif // _SMOOCH_TYPE_EQUALS_H

