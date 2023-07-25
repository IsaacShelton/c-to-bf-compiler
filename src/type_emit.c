
#include <stdio.h>
#include "../include/type_emit.h"
#include "../include/builtin_types.h"
#include "../include/storage.h"
#include "../include/type_print.h"

u32 type_sizeof_or_max(u32 type_index){
    if(type_index == u0_type) return 0;
    if(type_index == u8_type) return 1;
    if(type_index == u16_type) return 2;

    printf("\nerror: Type '");
    type_print(types[type_index]);
    printf("' does not exist\n");
    return -1;
}

