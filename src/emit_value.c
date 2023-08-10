
#include <stdio.h>
#include "../include/emit_value.h"
#include "../include/emit_context.h"

u0 emit_u8(u8 value){
    printf("[-]%d+>", value);
    emit_context.current_cell_index++;
}

u0 emit_u32(u32 value){
    emit_u8(value >> 24);
    emit_u8((value >> 16) & 0xFF);
    emit_u8((value >> 8) & 0xFF);
    emit_u8(value & 0xFF);
}

