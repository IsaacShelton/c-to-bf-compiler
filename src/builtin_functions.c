
#include <stdio.h>
#include "../include/builtin_functions.h"
#include "../include/builtin_types.h"
#include "../include/storage.h"

static ErrorCode add_put_function(){
    u8 raw_function_name[16] = "put";

    u32 name = aux_cstr_alloc(raw_function_name);
    if(name >= AUX_CAPACITY) return 1;

    u8 raw_parameter_name[16] = "c";
    u32 parameter_name = aux_cstr_alloc(raw_parameter_name);

    u32 operands = add_operands2(u8_type, parameter_name);
    if(operands >= OPERANDS_CAPACITY) return 1;

    u32 argument = add_statement_from_new((Expression){
        .kind = EXPRESSION_DECLARE,
        .line = u24_pack(0),
        .ops = operands,
    });

    u32 implementation = add_statement_from_new((Expression){
        .kind = EXPRESSION_IMPLEMENT_PUT,
        .line = u24_pack(0),
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
        .line = u24_pack(0),
    };

    if(add_function(function) >= FUNCTIONS_CAPACITY) return 1;
    return 0;
}

static ErrorCode add_printu1_function(){
    u8 raw_function_name[16] = "printu1";

    u32 name = aux_cstr_alloc(raw_function_name);
    if(name >= AUX_CAPACITY) return 1;

    u8 raw_parameter_name[16] = "value";
    u32 parameter_name = aux_cstr_alloc(raw_parameter_name);

    u32 operands = add_operands2(u1_type, parameter_name);
    if(operands >= OPERANDS_CAPACITY) return 1;

    u32 argument = add_statement_from_new((Expression){
        .kind = EXPRESSION_DECLARE,
        .line = u24_pack(0),
        .ops = operands,
    });

    u32 implementation = add_statement_from_new((Expression){
        .kind = EXPRESSION_IMPLEMENT_PRINTU1,
        .line = u24_pack(0),
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
        .line = u24_pack(0),
    };

    if(add_function(function) >= FUNCTIONS_CAPACITY) return 1;
    return 0;
}

static ErrorCode add_printu8_function(){
    u8 raw_function_name[16] = "printu8";

    u32 name = aux_cstr_alloc(raw_function_name);
    if(name >= AUX_CAPACITY) return 1;

    u8 raw_parameter_name[16] = "value";
    u32 parameter_name = aux_cstr_alloc(raw_parameter_name);

    u32 operands = add_operands2(u8_type, parameter_name);
    if(operands >= OPERANDS_CAPACITY) return 1;

    u32 argument = add_statement_from_new((Expression){
        .kind = EXPRESSION_DECLARE,
        .line = u24_pack(0),
        .ops = operands,
    });

    u32 implementation = add_statement_from_new((Expression){
        .kind = EXPRESSION_IMPLEMENT_PRINTU8,
        .line = u24_pack(0),
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
        .line = u24_pack(0),
    };

    if(add_function(function) >= FUNCTIONS_CAPACITY) return 1;
    return 0;
}

static ErrorCode add_get_function(){
    u8 raw_function_name[16] = "get";

    u32 name = aux_cstr_alloc(raw_function_name);
    if(name >= AUX_CAPACITY) return 1;

    u32 implementation = add_statement_from_new((Expression){
        .kind = EXPRESSION_IMPLEMENT_GET,
        .line = u24_pack(0),
        .ops = 0,
    });

    if(implementation >= STATEMENTS_CAPACITY) return 1;

    Function function = (Function){
        .name = name,
        .arity = 0,
        .return_type = u8_type,
        .begin = implementation,
        .num_stmts = 1,
        .is_recursive = false,
        .line = u24_pack(0),
    };

    if(add_function(function) >= FUNCTIONS_CAPACITY) return 1;
    return 0;
}

ErrorCode add_builtin_functions(){
    if(add_put_function()) return 1;
    if(add_printu1_function()) return 1;
    if(add_printu8_function()) return 1;
    if(add_get_function()) return 1;
    return 0;
}

