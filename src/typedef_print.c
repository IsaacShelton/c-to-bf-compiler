
#include <stdio.h>
#include "../include/storage.h"
#include "../include/typedef.h"
#include "../include/expression_print.h"


static u0 typedef_struct_print(TypeDef def){
    printf("typedef struct {\n");

    for(u32 i = 0; i < def.num_fields; i++){
        printf("    ");
        expression_print(expressions[statements[def.begin + i]]);
        printf(";\n");
    }

    printf("} ");
    print_aux_cstr(def.name);
    printf(";\n");
}

u0 typedef_print(TypeDef def){
    switch(def.kind){
    case TYPEDEF_STRUCT:
        typedef_struct_print(def);
        break;
    default:
        printf("<unknown typdef>");
    }
}

