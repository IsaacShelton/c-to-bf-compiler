
#include <stdio.h>
#include "../include/expression_emit.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/emit.h"

int expression_emit(Expression expression, int current_cell_index){
    switch(expression.kind){
    case EXPRESSION_DECLARE:
        // Nothing to do
        return 0;
    case EXPRESSION_PRINT:
        emit_print_aux_cstr(expression.ops);
        return 0;
    default:
        printf("error: Unknown statement kind %d\n", expression.kind);
        return 1;
    }
}

