
#include <stdio.h>
#include <string.h>
#include "../include/type_emit.h"
#include "../include/builtin_types.h"
#include "../include/storage.h"
#include "../include/type_print.h"

u32 type_sizeof_or_max(u32 type_index, u24 line_on_error){
    u32 name = types[type_index].name;
    u32 size;

    if(aux_cstr_equals_u0(name)){
        size = 0;
    } else if(aux_cstr_equals_u8(name) || aux_cstr_equals_u1(name)){
        size = 1;
    } else if(aux_cstr_equals_u16(name)){
        size = 2;
    } else if(aux_cstr_equals_u32(name)){
        size = 4;
    } else {
        u32 found_typedef = find_typedef(name);
        if(found_typedef >= TYPEDEFS_CAPACITY){
            type_does_not_exist(types[type_index], line_on_error);
            return -1;
        }

        size = typedefs[found_typedef].computed_size;

        if(size == -1){
            printf("\nerror on line %d: Type '", u24_unpack(line_on_error));
            type_print(types[type_index]);
            printf("' doesn't have its size computed yet\n");
            return -(u32) 1;
        }
    }

    u32 dims[4];
    memcpy(dims, dimensions[types[type_index].dimensions], sizeof dims);

    for(u8 i = 0; i < 4 && dims[i] != 0; i++){
        size *= dims[i];
    }

    return size;
}

