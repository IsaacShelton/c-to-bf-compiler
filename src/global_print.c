
#include <stdio.h>
#include "../include/global_print.h"
#include "../include/storage.h"
#include "../include/type_print.h"

void global_print(Global global){
    type_print(types[global.type]);
    printf(" ");
    print_aux_cstr(global.name);
    printf(";");
}

