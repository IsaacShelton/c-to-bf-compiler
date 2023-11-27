
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
        emit_context.enable_stack = true;
    }

    // Emit globals
    u32 global_variables_cells = 0;
    for(u32 i = 0; i < num_globals; i++){
        u32 size = type_sizeof_or_max(globals[i].type, globals[i].line);
        if(size == -1) return 1;

        global_variables_cells += size;
    }

    // Emit stack and stack pointer
    // Stack size + size of stack pointer
    u32 stack_size = DEFAULT_STACK_SIZE;
    u32 stack_storage = emit_context.enable_stack ? stack_size + 4 : 0;

    // Assume that tape starts as zeroed
    printf("%d>", global_variables_cells + stack_storage);

    u32 start_cell_index = global_variables_cells + stack_storage;

    // Position stack and stack pointer
    emit_context.stack_pointer = global_variables_cells + stack_size;
    emit_context.stack_begin = global_variables_cells;
    emit_context.current_cell_index = start_cell_index;

    if(emit_context.enable_stack){
        emit_stack_driver_pre();
    }

    if(!emit_context.enable_stack){
        // Emit main function
        if(function_emit(main_function_index, start_cell_index, start_cell_index)){
            return 1;
        }
    }

    if(emit_context.enable_stack){
        emit_stack_driver_post();
    }

    return 0;
}

