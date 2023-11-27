
#include <stdio.h>
#include "../include/emit.h"
#include "../include/emit_context.h"
#include "../include/storage.h"
#include "../include/stack_driver.h"
#include "../include/function_emit.h"
#include "../include/type_emit.h"

void emit_stack_pointer(){
    copy_cells_static(emit_settings.stack_pointer, 4);
}

void emit_set_stack_pointer(){
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_settings.stack_pointer, 4);
}

void emit_stack_driver_pre(u32 main_function_index){
    // Enable stack overflow checks
    emit_settings.stack_overflow_checks = true;
    emit_settings.stack_overflow_message = aux_cstr_alloc((u8*) "---- STACK OVERFLOW!!! ----\n");

    // Create stack pointer
    emit_settings.stack_pointer = emit_context.current_cell_index;
    emit_u32(8);

    // Add exit (basicblock 0) to stack
    emit_u32(0);
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_settings.stack_begin, 4);

    // Add entry point to stack
    emit_u32(basicblock_id_for_function(main_function_index));
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_settings.stack_begin + 4, 4);

    // Do while
    printf("[-]+[");

    // Read next basicblock to execute from stack
    emit_stack_pop_n(4);

    // Debug print new basicblock id
    /*
    dupe_cells(4);
    printf("3<");
    emit_printu8();
    printf("[-]++++++++++.");
    printf("<");
    emit_context.current_cell_index -= 4;
    */

    // Reserve number of basicblocks for exit basicblock as well as all functions
    emit_settings.next_basicblock_id = 1 + num_functions;

    // Retain stack driver start position
    emit_settings.stack_driver_position = emit_context.current_cell_index;

    // Switch basicblock id

    // (closed via emit_stack_driver_post)

    /*
    emit_start_basicblock(basicblock_id_for_function(main_function_index));
        printf("[-]>[-]>[-]>[-]>[-]>[-]>6<");
        printf(">++++++++[<+++++++++>-]<.>++++[<+++++++>-]<+.+++++++..+++.>>++++++[<+++++++>-]<++.------------.>++++++[<+++++++++>-]<+.<.+++.------.--------.>>>++++[<++++++++>-]<+.<<");
        printf("[-]++++++++++.");

        emit_u32(100);
        emit_stack_push_n(4);
    emit_end_basicblock();

    emit_start_basicblock(100);
        u32 aux_cstr = aux_cstr_alloc((u8*) "Bye world!!!\n");
        emit_print_aux_cstr(aux_cstr);
        emit_u32(basicblock_id_for_function(main_function_index));
        emit_stack_push_n(4);
    emit_end_basicblock();
    */
}

u32 basicblock_id_for_function(u32 function_id){
    return function_id + 1;
}

void emit_start_basicblock(u32 basicblock_id){
    dupe_cells(4);
    emit_u32(basicblock_id);
    emit_eq_u32();
    printf("<");
    emit_context.current_cell_index--;
    printf("[");
}

void emit_end_basicblock(){
    printf("[-]]");
}

void emit_stack_driver_post(){
    // Keep looping until basicblock id is zero
    emit_u32(0);
    emit_neq_u32();
    printf("<");
    printf("]");
}

void emit_basicblock_pre(u32 basicblock_id){
    // Run basicblock if current basicblock id matches this basicblock id
    dupe_cells(4);
    emit_u32(basicblock_id);
    emit_eq_u32();
    printf("[");
}

void emit_basicblock_post(){
    if(emit_settings.stack_pointer != emit_settings.stack_driver_position){
        fprintf(stderr, "error: basicblock content corrupted stack driver position\n");
    }

    // End if
    printf("[-]]");
}

u32 emit_stack_driver_push_all(){
    u32 count = emit_context.current_cell_index - emit_settings.stack_driver_position;
    emit_stack_push_n(count);
    return count;
}

void emit_stack_driver_pop_all(){
    // Compute `stack_pointer - 4`
    copy_cells_static(emit_settings.stack_pointer, 4);
    emit_u32(4);
    emit_additive_u32(false);

    // Get number of local cells
    copy_cells_dynamic_u32(emit_settings.stack_begin, 4);

    /*
    i = 0;
    while(i != num_local_cells){
        stack_pointer();
        copy(i);
        emit_additive(true);
        copy_cell_dynamic_u32(emit_context.stack_begin, 4);
        i++;
    }
    */
}

void emit_stack_push_n(u32 num_cells){
    // data1 data2 data3 data4
    //                         ^

    if(num_cells != 0){
        if(emit_settings.stack_overflow_checks){
            emit_stack_pointer();
            emit_u32(num_cells);
            emit_additive_u32(true);
            emit_u32(emit_settings.stack_size);
            emit_gte_u32();
            printf("<");
            emit_context.current_cell_index--;
            printf("[");
                emit_print_aux_cstr(emit_settings.stack_overflow_message);
                printf("[-]+[]");
            printf("]");
        }

        // Move bytes onto stack
        emit_stack_pointer();
        printf("<");
        emit_context.current_cell_index--;
        move_cells_dynamic_u32(emit_settings.stack_begin, num_cells);

        // Increase stack pointer
        emit_stack_pointer();
        emit_u32(num_cells);
        emit_additive_u32(true);
        emit_set_stack_pointer();
    }
}

void emit_stack_pop_n(u32 num_cells){
    if(num_cells != 0){
        // Compute `stack_pointer - n`
        emit_stack_pointer();
        emit_u32(num_cells);
        emit_additive_u32(false);

        // Set stack pointer to `stack_pointer - num_cells`
        dupe_cells(4);
        emit_set_stack_pointer();

        // Read cells from stack
        copy_cells_dynamic_u32(emit_settings.stack_begin, num_cells);
    }
}

u32 emit_recursive_functions(){
    for(u32 function_i = 0; function_i < num_functions; function_i++){
        Function function = functions[function_i];

        if(!function.is_recursive){
            continue;
        }

        fprintf(stderr, "Generating recursive function %d (basicblock %d)\n", (int) function_i, (int) basicblock_id_for_function(function_i));
        printf("\n");

        emit_start_basicblock(basicblock_id_for_function(function_i));
            // Debug message
            /*
            printf("[-]>[-]>[-]>[-]>[-]>[-]>6<");
            printf(">++++++++[<+++++++++>-]<.>++++[<+++++++>-]<+.+++++++..+++.>>++++++[<+++++++>-]<++.------------.>++++++[<+++++++++>-]<+.<.+++.------.--------.>>>++++[<++++++++>-]<+.<<");
            printf("[-]++++++++++.");
            */

            if(emit_settings.stack_driver_position != emit_context.current_cell_index){
                int off_by = (int) emit_context.current_cell_index - (int) emit_settings.stack_driver_position;
                printf("\ninternal error on line %d: Failed to generate recursive function as final resting cell index does not match expected stack driver position (%d cells off)\n", u24_unpack(function.line), off_by);
                return 1;
            }

            u32 return_size = type_sizeof_or_max(function.return_type, function.line);
            if(return_size == -1) return 1;

            u32 args_size = function_args_size(function);
            if(args_size == -1) return 1;

            emit_stack_pop_n(return_size + args_size);
            function_emit(function_i, emit_context.current_cell_index - args_size, emit_context.current_cell_index);
            emit_stack_push_n(return_size);
        emit_end_basicblock();
    }

    return 0;
}

