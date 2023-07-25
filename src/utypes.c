
#include "../include/utypes.h"

u24 u24_pack(u32 value){
    u24 result;
    result.bytes[0] = (value >> 16) & 0xFF;
    result.bytes[1] = (value >> 8) & 0xFF;
    result.bytes[2] = value & 0xFF;
    return result;
}

u32 u24_unpack(u24 value){
    return ((u32) value.bytes[0] << 16)
         + ((u32) value.bytes[1] << 8)
         + ((u32) value.bytes[2]);
}

