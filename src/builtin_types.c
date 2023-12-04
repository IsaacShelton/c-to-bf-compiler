
#include "../include/builtin_types.h"
#include "../include/storage.h"

u32 u0_type;
u32 u1_type;
u32 u8_type;
u32 u16_type;
u32 u24_type;
u32 u32_type;

static u32 make_simple_type(u8 raw_name[16]){
    // Alloc string
    u32 name = aux_cstr_alloc(raw_name);
    if(name >= AUX_CAPACITY) return TYPES_CAPACITY;

    // Create type
    return add_type((Type){
        .name = name,
        .dimensions = (u32) 0,
    });
}

ErrorCode add_builtin_types(){
    // Create u0 type
    u8 u0_name[16] = "u0";
    u0_type = make_simple_type(u0_name);
    if(u0_type >= TYPES_CAPACITY) return 1;

    // Create u1 type
    u8 u1_name[16] = "u1";
    u1_type = make_simple_type(u1_name);
    if(u1_type >= TYPES_CAPACITY) return 1;
    
    // Create u8 type
    u8 u8_name[16] = "u8";
    u8_type = make_simple_type(u8_name);
    if(u8_type >= TYPES_CAPACITY) return 1;

    // Create u16 type
    u8 u16_name[16] = "u16";
    u16_type = make_simple_type(u16_name);
    if(u16_type >= TYPES_CAPACITY) return 1;

    // Create u24 type
    u8 u24_name[16] = "u24";
    u24_type = make_simple_type(u24_name);
    if(u24_type >= TYPES_CAPACITY) return 1;

    // Create u32 type
    u8 u32_name[16] = "u32";
    u32_type = make_simple_type(u32_name);
    if(u32_type >= TYPES_CAPACITY) return 1;

    return 0;
}

