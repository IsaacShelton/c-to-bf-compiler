
#include "../include/infer.h"
#include "../include/storage.h"
#include "../include/function_infer.h"
#include "../include/expression_infer.h"

u32 infer(){
    for(u32 i = 0; i < num_globals; i++){
        Global global = globals[i];

        if(global.initializer < EXPRESSIONS_CAPACITY){
            expression_infer(global.initializer, type_to_expression_kind(global.type));
        }
    }

    for(u32 i = 0; i < num_functions; i++){
        function_infer(i);
    }
    return 0;
}

