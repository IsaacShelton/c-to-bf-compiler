
#include "../include/function.h"
#include "../include/function_infer.h"
#include "../include/expression_infer.h"
#include "../include/storage.h"
#include "../include/emit_context.h"

u0 function_infer(u32 function_index){
    Function function = functions[function_index];

    for(u32 i = 0; i < function.num_stmts; i++){
        u32 statement = statements[function.begin + i];

        // For variable lookups
        emit_context.function = function_index;
        emit_context.current_statement = function.begin + i;

        expression_infer(statement, EXPRESSION_NONE);
    }
}

