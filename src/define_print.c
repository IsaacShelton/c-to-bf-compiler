
#include <stdio.h>
#include "../include/storage.h"
#include "../include/define_print.h"
#include "../include/expression_print.h"

u0 define_print(Define define){
    printf("#define ");
    print_aux_cstr(define.name);
    printf(" ");
    expression_print(expressions[define.expression]);
}

