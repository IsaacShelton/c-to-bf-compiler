
#ifndef _PECK_GLOBAL_H
#define _PECK_GLOBAL_H

#include "utypes.h"

typedef struct {
    u32 name;
    u32 type;
    u24 line;
    u32 initializer;
} Global;

#endif // _PECK_GLOBAL_H

