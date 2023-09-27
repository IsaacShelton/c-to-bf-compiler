
#include <stdio.h>
#include "../include/storage.h"
#include "../include/type_print.h"

u0 type_does_not_exist(Type type, u24 line){
    printf("\nerror on line %d: Type '", u24_unpack(line));
    type_print(type);
    printf("' does not exist\n");
}

