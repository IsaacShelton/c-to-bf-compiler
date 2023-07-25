
#include "../include/function_infer.h"
#include "../include/expression_infer.h"
#include "../include/storage.h"

u0 function_infer(Function function){
    for(u32 i = 0; i < function.num_stmts; i++){
        u32 statement = statements[function.begin + i];
        expression_infer(statement, EXPRESSION_NONE);
    }
}

