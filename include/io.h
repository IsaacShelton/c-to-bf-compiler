
#ifndef _BRAINLOVE_IO_H
#define _BRAINLOVE_IO_H

#include <stdio.h>

static inline void put(char c){
    putchar(c);
}

static inline char get(void){
    char c = getchar();
    return c < 0 ? 0 : c;
}

static inline void putbuf(char *buffer, size_t length){
    for(int i = 0; i < length; i++){
        put(buffer[i]);
    }
}

#endif // _BRAINLOVE_IO_H

