
#include <stdio.h>
#include "../include/function.h"
#include "../include/type_emit.h"
#include "../include/storage.h"

u32 function_args_size(Function function){
    u32 total_size = 0;

    for(u8 i = 0; i < function.arity; i++){
        Expression arg = expressions[statements[function.begin + (u32) i]];

        if(arg.kind != EXPRESSION_DECLARE){
            printf("\ninternal error on line %d: Failed to compute arguments size\n", u24_unpack(function.line));
            return -1;
        }

        u32 arg_size = type_sizeof_or_max(operands[arg.ops], arg.line);
        if(arg_size == -1) return -1;

        total_size += arg_size;
    }

    return total_size;
}

