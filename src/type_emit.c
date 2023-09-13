
#include <stdio.h>
#include <string.h>
#include "../include/type_emit.h"
#include "../include/builtin_types.h"
#include "../include/storage.h"
#include "../include/type_print.h"

u32 type_sizeof_or_max(u32 type_index){
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
            printf("\nerror: Type '");
            type_print(types[type_index]);
            printf("' does not exist\n");
            return -1;
        }

        size = typedefs[found_typedef].computed_size;

        if(size == -1){
            printf("\nerror: Type '");
            type_print(types[type_index]);
            printf("' doesn't have its size computed yet\n");
            return -1;
        }
    }

    u32 dims[4];
    memcpy(dims, dimensions[types[type_index].dimensions], sizeof(u32) * 4);

    for(u8 i = 0; i < 4 && dims[i]; i++){
        size *= dims[i];
    }

    return size;
}

