
#ifndef _PECK_IO_H
#define _PECK_IO_H

#include <stdio.h>
#include <stdlib.h>
#include "utypes.h"

static inline u0 put(u8 c){
    putchar(c);
}

static inline u8 get(void){
    char c = getc(stdin);
    return c != EOF ? c : 0;
}

#endif // _PECK_IO_H

