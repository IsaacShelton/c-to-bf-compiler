
#include <stdio.h>
#include "../include/expression_print.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/type_print.h"

void expression_print(Expression expression){
    switch(expression.kind){
    case EXPRESSION_DECLARE:
        type_print(types[operands[expression.ops]]);
        printf(" ");
        print_aux_cstr(operands[expression.ops + 1]);
        break;
    case EXPRESSION_PRINT:
        printf("print(\"");
        print_aux_cstr(expression.ops);
        printf("\")");
        break;
    default:
        printf("<unknown expression>");
    }
}

