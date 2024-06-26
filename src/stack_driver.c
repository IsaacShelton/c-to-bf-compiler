
#include <stdio.h>
#include "../include/emit.h"
#include "../include/emit_context.h"
#include "../include/storage.h"
#include "../include/stack_driver.h"
#include "../include/function_emit.h"
#include "../include/type_emit.h"

u0 emit_stack_pointer(){
    copy_cells_static(emit_settings.stack_pointer, 4);
}

u0 emit_set_stack_pointer(){
    move_cells_static(emit_settings.stack_pointer, 4);
}

u0 emit_stack_driver_pre(u32 main_function_index){
    // Enable stack overflow checks
    emit_settings.stack_overflow_checks = true;
    emit_settings.stack_overflow_message = aux_cstr_alloc((u8*) "---- STACK OVERFLOW!!! ----\n");

    // Initialize `in_basicblock`
    emit_settings.in_basicblock = false;

    // Create stack pointer
    emit_settings.stack_pointer = emit_context.current_cell_index;
    emit_u32(8);

    // Add exit (basicblock 0) to stack
    emit_u32(0);
    move_cells_static(emit_settings.stack_begin, 4);

    // Add entry point to stack
    emit_u32(basicblock_id_for_function(main_function_index));
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
    printf("[-]++++++++++++++++++++++++++++++++.");
    printf("<");
    emit_context.current_cell_index -= 4;

    emit_stack_pointer();
    emit_printu8();
    printf("[-]++++++++++++++++++++++++++++++++.");
    printf("<");
    emit_printu8();
    printf("[-]++++++++++++++++++++++++++++++++.");
    printf("<");
    emit_printu8();
    printf("[-]++++++++++++++++++++++++++++++++.");
    printf("<");
    emit_printu8();
    printf("[-]++++++++++++++++++++++++++++++++.");
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

u0 emit_start_basicblock(u32 basicblock_id){
    dupe_cells(4);
    emit_u32(basicblock_id);
    emit_eq_u32();
    printf("<");
    emit_context.current_cell_index--;
    printf("[");

    emit_settings.in_basicblock = true;
}

u0 emit_end_basicblock(){
    printf("[-]]");
    emit_settings.in_basicblock = false;
}

u0 emit_start_basicblock_landing(u32 basicblock_id, u32 num_cells_to_pop){
    emit_start_basicblock(basicblock_id);
    emit_stack_pop_n(num_cells_to_pop);
}

u32 emit_jump(u32 target_basicblock_id){
    u32 pushed = emit_stack_driver_push_all();
    emit_u32(target_basicblock_id);
    emit_stack_push_n(4);
    return pushed;
}

u32 emit_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells){
    u32 pushed = emit_jump(target_basicblock_id);

    if(pushed != expected_pushed_cells){
        fprintf(stderr, "internal error: emit_jump_compatible - tried to do incompatible jump (%d vs %d)\n", pushed, expected_pushed_cells);
    }

    return pushed;
}

u32 emit_end_basicblock_jump(u32 target_basicblock_id){
    u32 pushed = emit_jump(target_basicblock_id);

    emit_end_basicblock();
    return pushed;
}

u0 emit_end_basicblock_jump_compatible(u32 target_basicblock_id, u32 expected_pushed_cells){
    u32 amount = emit_end_basicblock_jump(target_basicblock_id);

    if(amount != expected_pushed_cells){
        fprintf(stderr, "internal error: emit_end_basicblock_jump_compatible - tried to end basicblock via incompatible jump to %d (%d vs %d)\n", target_basicblock_id, amount, expected_pushed_cells);

        if(emit_context.function < FUNCTIONS_CAPACITY){
            printf("\n  Inside of function: ");
            print_aux_cstr(functions[emit_context.function].name);
            printf("\n");
        }
    }
}

u32 emit_end_basicblock_jump_conditional(u32 then_basicblock_id, u32 else_basicblock_id){
    // condition
    //           ^

    // Allocate 'should_run_else' cell
    printf("[-]+");

    // Go to condition cell
    printf("<");
    emit_context.current_cell_index--;

    u32 condition_cell = emit_context.current_cell_index;

    // If condition
    printf("[");
        u32 pushed = emit_jump(then_basicblock_id);

        // Go back to 'condition' cell
        printf("%d>", condition_cell - emit_context.current_cell_index);
        emit_context.current_cell_index = condition_cell;

        // Zero 'should_run_else' cell
        printf(">[-]<");

    // End if
    printf("[-]]");

    // Go to 'should_run_else' cell
    printf(">");
    emit_context.current_cell_index++;

    u32 else_cell = emit_context.current_cell_index;

    // If 'should_run_else' cell
    printf("[");
        printf("<");
        emit_context.current_cell_index -= 1;

        (u0) emit_jump_compatible(else_basicblock_id, pushed);

        // Go back to 'condition' cell
        printf("%d>", else_cell - emit_context.current_cell_index);
        emit_context.current_cell_index = else_cell;

    // End if
    printf("[-]]");

    // Go to stack driver position
    printf("%d<", emit_context.current_cell_index - emit_settings.stack_driver_position);
    emit_context.current_cell_index = emit_settings.stack_driver_position;

    // End basicblock
    emit_end_basicblock();
    return pushed;
}

u0 emit_end_basicblock_jump_to(JumpContext context){
    u32 has = emit_context.current_cell_index - emit_settings.stack_driver_position;
    u32 remove = has - context.num_cells_input;

    if(remove != 0){
        printf("%d<", remove);
        emit_context.current_cell_index -= remove;
    }

    emit_end_basicblock_jump_compatible(context.basicblock_id, context.num_cells_input);
}

u0 emit_stack_driver_post(){
    // Keep looping until basicblock id is zero
    emit_u32(0);
    emit_neq_u32();
    printf("<");
    printf("]");
}

u0 emit_basicblock_pre(u32 basicblock_id){
    // Run basicblock if current basicblock id matches this basicblock id
    dupe_cells(4);
    emit_u32(basicblock_id);
    emit_eq_u32();
    printf("[");
}

u0 emit_basicblock_post(){
    if(emit_settings.stack_pointer != emit_settings.stack_driver_position){
        fprintf(stderr, "error: basicblock content corrupted stack driver position\n");
    }

    // End if
    printf("[-]]");
}

u32 emit_stack_driver_push_all(){
    u32 count = emit_context.current_cell_index - emit_settings.stack_driver_position;
    emit_stack_push_n(count);

    if(emit_context.current_cell_index != emit_settings.stack_driver_position){
        fprintf(stderr, "internal error: emit_stack_driver_push_all() failed to pushed all\n");
    }

    return count;
}

u0 emit_stack_driver_pop_all(){
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

u0 emit_stack_push_n(u32 num_cells){
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
        move_cells_dynamic_u32(emit_settings.stack_begin, num_cells);

        // Increase stack pointer
        emit_stack_pointer();
        emit_u32(num_cells);
        emit_additive_u32(true);
        emit_set_stack_pointer();
    }
}

u0 emit_stack_pop_n(u32 num_cells){
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

        if(emit_settings.stack_driver_position != emit_context.current_cell_index){
            int off_by = (int) emit_context.current_cell_index - (int) emit_settings.stack_driver_position;
            fprintf(stderr, "internal error on line %d: Failed to generate recursive function as final resting cell index does not match expected stack driver position (%d cells off)\n", u24_unpack(function.line), off_by);
            return 1;
        }

        u32 args_size = function_args_size(function);
        if(args_size == -1) return 1;

        /*
        fprintf(stderr, "Emitting Function %d (", function_i);
        print_aux_cstr_err(functions[function_i].name);
        fprintf(stderr, ")\n");
        */

        emit_start_basicblock_landing(basicblock_id_for_function(function_i), args_size);
        if(function_emit(function_i, emit_settings.stack_driver_position, emit_context.current_cell_index) != 0){
            return 1;
        }
        if(emit_settings.in_basicblock){
            emit_end_basicblock();
        }
    }

    return 0;
}
