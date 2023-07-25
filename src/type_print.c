
#include <stdio.h>
#include "../include/type_print.h"
#include "../include/storage.h"

u0 type_print(Type type){
    print_aux_cstr(type.name);

    for(u32 i = 0; i < 4; i++){
        u32 dim = dimensions[type.dimensions][i];
        if(dim == 0) break;

        printf("[%d]", dim);
    }
}

