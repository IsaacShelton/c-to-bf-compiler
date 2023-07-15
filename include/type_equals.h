
#ifndef _BRAINLOVE_TYPE_EQUALS_H
#define _BRAINLOVE_TYPE_EQUALS_H

#include "../include/type.h"

bool type_equals(Type a, Type b){
    return a.dimensions == b.dimensions && aux_cstr_equals(a.name, b.name);
}

#endif // _BRAINLOVE_TYPE_EQUALS_H

