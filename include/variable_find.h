
#ifndef _BRAIN_VARIABLE_FIND_H
#define _BRAIN_VARIABLE_FIND_H

#include <stdbool.h>

typedef struct {
    int name; // name index in `aux`
    int type; // type index in `types`
    int depth; // scope depth, depth of 0 means global, anything else means in a scope
    int declaration; // index of declaration, (global index if global otherwise statement index)
    bool defined; // whether defined
} Variable;

Variable variable_find(int function_beginning, int current_statement, int name);

#endif // _BRAIN_VARIABLE_FIND_H

