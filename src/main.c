
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
#include "../include/typedef_print.h"
#include "../include/infer.h"
#include "../include/mark_recursive_functions.h"
#include "../include/compute_typedef_sizes.h"
#include "../include/prelex.h"
#include "../include/type_emit.h"
#include "../include/lex_context.h"
#include "../include/emit_context.h"
#include "../include/stack_driver.h"

int main(void){
    if(lex()) return 1;

    if(lex_peck_print_tokens){
        for(u32 i = 0; i < num_tokens; i++){
            token_print(tokens[i], true);
            printf("\n");
        }
        printf("\n");
    }

    if(lex_peck_print_lexed_construction){
        print_lexed_construction();
        return 0;
    }

    if(parse()) return 1;
    if(compute_typedef_sizes()) return 1;
    if(infer()) return 1;
    if(mark_recursive_functions()) return 1;

    /*
    for(u32 i = 0; i < num_typedefs; i++){
        typedef_print(typedefs[i]);
        printf("\n");
    }
    */

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

    // Determine if recursion necessary
    if(functions[main_function_index].is_recursive){
        emit_settings.enable_stack = true;
    }

    // Emit globals
    u32 global_variables_cells = 0;
    for(u32 i = 0; i < num_globals; i++){
        u32 size = type_sizeof_or_max(globals[i].type, globals[i].line);
        if(size == -1) return 1;

        global_variables_cells += size;
    }

    // Calculate stack footprint
    u32 stack_footprint = emit_settings.enable_stack ? DEFAULT_STACK_SIZE : 0;

    // Allocate global variables and stack memory
    printf("%d>", global_variables_cells + stack_footprint);
    emit_settings.stack_begin = global_variables_cells;

    // Calculate starting cell index
    u32 start_cell_index = global_variables_cells + stack_footprint;
    emit_context.current_cell_index = start_cell_index;

    // If stack is disabled, just emit the `main` function
    if(!emit_settings.enable_stack){
        return function_emit(main_function_index, start_cell_index, start_cell_index);
    }

    // Otherwise, continue as normal and use stack
    emit_stack_driver_pre(main_function_index);
    if(emit_recursive_functions()) return 1;
    emit_stack_driver_post();
    return 0;
}

