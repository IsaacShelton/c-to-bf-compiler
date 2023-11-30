
#include <stdio.h>
#include "../include/storage.h"
#include "../include/typedef.h"
#include "../include/expression_print.h"
#include "../include/type_print.h"

static u0 typedef_struct_print(TypeDef def){
    printf("typedef struct {\n");

    for(u32 i = 0; i < def.num_fields; i++){
        printf("    ");
        expression_print(expressions[statements[def.begin + i]]);
        printf(";\n");
    }

    printf("} ");
    print_aux_cstr(def.name);
    printf(";");
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

u0 type_alias_print(TypeAlias alias){
    printf("typedef ");
    type_print(types[alias.rewritten_type]);
    printf(" ");
    print_aux_cstr(alias.name);
    printf(";");
}

