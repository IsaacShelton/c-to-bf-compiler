
#ifndef _SMOOCH_UTYPES_H
#define _SMOOCH_UTYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef void u0;
typedef bool u1;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct { u8 bytes[3]; } u24;
u24 u24_pack(u32 value);
u32 u24_unpack(u24 value);

#endif // _SMOOCH_UTYPES_H

