
#include <stdio.h>
#include "../include/expression_print.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/type_print.h"

u0 expression_print(Expression expression){
    switch(expression.kind){
    case EXPRESSION_DECLARE:
        type_print(types[operands[expression.ops]]);
        printf(" ");
        print_aux_cstr(operands[expression.ops + 1]);
        break;
    case EXPRESSION_PRINT:
        printf("print(\"");
        print_aux_cstr_escaped(expression.ops);
        printf("\")");
        break;
    case EXPRESSION_CALL: {
            u32 name = operands[expression.ops];
            u32 arity = operands[expression.ops + 1];
            print_aux_cstr(name);
            printf("(");
            for(u32 i = 0; i < arity; i++){
                expression_print(expressions[operands[expression.ops + 2 + i]]);
                if(i + 1 != arity){
                    printf(", ");
                }
            }
            printf(")");
        }
        break;
    case EXPRESSION_IMPLEMENT_PUT:
        printf("<implementation of put>");
        break;
    case EXPRESSION_INT:
        printf("%d", expression.ops);
        break;
    case EXPRESSION_U8:
        printf("%du8", expression.ops);
        break;
    case EXPRESSION_ASSIGN:
        print_aux_cstr(operands[expression.ops]);
        printf(" = ");
        expression_print(expressions[operands[expression.ops + 1]]);
        break;
    default:
        printf("<unknown expression>");
    }
}

