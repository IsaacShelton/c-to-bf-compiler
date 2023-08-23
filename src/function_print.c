
#include <stdio.h>
#include "../include/function_print.h"
#include "../include/function.h"
#include "../include/storage.h"
#include "../include/type_print.h"
#include "../include/expression_print.h"

static u0 indent(u32 indentation){
    for(u32 i = 0; i < indentation; i++){
        printf("    ");
    }
}

static u0 print_body(u32 start_i, u32 stop_i, u32 indentation){
    for(u32 i = start_i; i < stop_i; i++){
        printf(" %02d ", i);
        if(indentation > 1){
            indent(indentation - 1);
        }

        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_IF:
        case EXPRESSION_IF_ELSE: {
                u32 when_true_len = operands[expression.ops + 1];

                printf("if(");
                expression_print(expressions[operands[expression.ops]]);
                printf("){\n");
                print_body(i + 1, i + when_true_len + 1, indentation + 1);
                i += when_true_len;

                indent(indentation);
                printf("}");

                if(expression.kind == EXPRESSION_IF_ELSE){
                    u32 when_false_len = operands[expression.ops + 2];
                    printf(" else {\n");
                    print_body(i + 1, i + when_false_len + 1, indentation + 1);
                    i += when_false_len;
                    indent(indentation);
                    printf("}");
                }

                printf("\n");
            }
            break;
        default:
            expression_print(expression);
            printf(";\n");
        }
    }
}

u0 function_print(Function function){
    type_print(types[function.return_type]);
    printf(" ");
    print_aux_cstr(function.name);
    printf("(");

    u32 i = 0;

    while(i < function.num_stmts && i < function.arity){
        expression_print(expressions[statements[function.begin + i]]);
        i++;

        if(i != function.arity){
            printf(", ");
        }
    }

    printf("){\n");
    u32 start_i = function.begin + i;
    u32 stop_i = function.begin + function.num_stmts;
    print_body(start_i, stop_i, 1);
    printf("}");
}

