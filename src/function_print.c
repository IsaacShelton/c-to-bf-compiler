
#include <stdio.h>
#include "../include/function_print.h"
#include "../include/function.h"
#include "../include/storage.h"
#include "../include/type_print.h"
#include "../include/expression_print.h"

void function_print(Function function){
    type_print(types[function.return_type]);
    printf(" ");
    print_aux_cstr(function.name);
    printf("(");
    printf("){\n");

    for(int i = 0; i < function.num_stmts; i++){
        printf("    ");
        expression_print(expressions[statements[function.begin + i]]);
        printf(";\n");
    }

    printf("}");
}

