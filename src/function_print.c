
#include <stdio.h>
#include "../include/function_print.h"
#include "../include/function.h"
#include "../include/storage.h"
#include "../include/type_print.h"
#include "../include/expression_print.h"

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

    while(i < function.num_stmts){
        printf("    ");
        expression_print(expressions[statements[function.begin + i]]);
        printf(";\n");
        i++;
    }

    printf("}");
}

