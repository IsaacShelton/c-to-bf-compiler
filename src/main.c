
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/config.h"
#include "../include/token.h"
#include "../include/io.h"
#include "../include/storage.h"
#include "../include/lex.h"
#include "../include/parse.h"
#include "../include/token_print.h"
#include "../include/function_print.h"
#include "../include/function_emit.h"
#include "../include/infer.h"
#include "../include/mark_recursive_functions.h"

int main(void){
    if(lex()) return 1;

    /*
    for(u32 i = 0; i < num_tokens; i++){
        token_print(tokens[i], true);
        printf("\n");
    }
    printf("\n");
    */

    if(parse()) return 1;
    if(infer()) return 1;
    if(mark_recursive_functions()) return 1;

    /*
    for(u32 i = 0; i < num_functions; i++){
        function_print(functions[i]);
        printf("\n");
    }
    */

    // Write

    // Find main function
    u32 main_function_index = FUNCTIONS_CAPACITY;
    for(u32 i = 0; i < FUNCTIONS_CAPACITY; i++){
        if(aux_cstr_equals_main(functions[i].name)){
            main_function_index = i;
            break;
        }
    }

    // Require main function
    if(main_function_index == FUNCTIONS_CAPACITY){
        printf("\nerror: No main function\n");
        return 1;
    }

    if(functions[main_function_index].is_recursive){
        printf("\nerror: Recursion not yet supported\n");
        return 1;
    }

    // Emit main function
    return function_emit(main_function_index, 0, 0);
}

