
#include <stdio.h>
#include "../include/type_print.h"
#include "../include/storage.h"

void type_print(Type type){
    print_aux_cstr(type.name);

    for(int i = 0; i < 4; i++){
        int dim = dimensions[type.dimensions][i];
        if(dim == 0) break;

        printf("[%d]", dim);
    }
}

