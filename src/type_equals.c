
#include <stdio.h>
#include "../include/type_equals.h"
#include "../include/storage.h"

bool type_equals(Type a, Type b){
    return a.dimensions == b.dimensions && aux_cstr_equals(a.name, b.name);
}

