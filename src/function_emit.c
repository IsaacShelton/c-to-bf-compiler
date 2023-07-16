
#include "../include/function_emit.h"
#include "../include/function.h"
#include "../include/storage.h"
#include "../include/expression_emit.h"

int function_emit(Function function, int current_cell_index){
    for(int i = 0; i < function.num_stmts; i++){
        if(expression_emit(expressions[statements[function.begin + i]], 0)){
            return 1;
        }
    }
    return 0;
}
