
#ifndef _PECK_FUNCTION_EMIT
#define _PECK_FUNCTION_EMIT

#include "utypes.h"
#include "function.h"

ErrorCode function_emit(u32 function_index, u32 start_function_cell_index, u32 start_current_cell_index);
u32 function_get_header_size_or_max(u32 function_index);

#endif // _PECK_FUNCTION_EMIT

