
#ifndef _SMOOCH_IO_H
#define _SMOOCH_IO_H

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

static inline void putbuf(u8 *buffer, u32 length){
    for(u32 i = 0; i < length; i++){
        put(buffer[i]);
    }
}

#endif // _SMOOCH_IO_H

