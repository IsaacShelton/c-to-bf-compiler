
#include <stdio.h>
#include "../include/builtin_functions.h"
#include "../include/builtin_types.h"
#include "../include/storage.h"

static u32 add_put_function(u32 u0_type, u32 u8_type){
    u8 raw_function_name[16] = "put";

    u32 name = aux_cstr_alloc(raw_function_name);
    if(name >= AUX_CAPACITY) return 1;

    u8 raw_parameter_name[16] = "c";
    u32 parameter_name = aux_cstr_alloc(raw_parameter_name);

    u32 operands = add_operands2(u8_type, parameter_name);
    if(operands >= OPERANDS_CAPACITY) return 1;

    u32 argument = add_statement_from_new((Expression){
        .kind = EXPRESSION_DECLARE,
        .ops = operands,
    });

    u32 implementation = add_statement_from_new((Expression){
        .kind = EXPRESSION_IMPLEMENT_PUT,
        .ops = 0,
    });

    if(implementation >= STATEMENTS_CAPACITY) return 1;

    Function function = (Function){
        .name = name,
        .arity = 1,
        .return_type = u0_type,
        .begin = argument,
        .num_stmts = 2,
        .is_recursive = false,
    };

    if(add_function(function)) return 1;
    return 0;
}

u32 add_builtin_functions(){
    // Create put function
    if(add_put_function(u0_type, u8_type)) return 1;

    return 0;
}

