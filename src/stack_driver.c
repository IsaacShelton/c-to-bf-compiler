
#include <stdio.h>
#include "../include/emit.h"
#include "../include/emit_context.h"
#include "../include/storage.h"
#include "../include/stack_driver.h"

void emit_stack_pointer(){
    copy_cells_static(emit_context.stack_pointer, 4);
}

void emit_set_stack_pointer(){
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_context.stack_pointer, 4);
}

void emit_stack_driver_pre(u32 main_function_index){
    // Create stack pointer
    emit_context.stack_pointer = emit_context.current_cell_index;
    emit_u32(8);

    // Retain stack driver start position
    emit_context.stack_driver_position = emit_context.current_cell_index;

    // Add exit (basicblock 0) to stack
    emit_u32(0);
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_context.stack_begin, 4);

    // Add entry point to stack
    emit_u32(basicblock_id_for_function(main_function_index));
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_context.stack_begin + 4, 4);

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
    emit_context.next_basicblock_id = 1 + num_functions;

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
    if(emit_context.stack_pointer != emit_context.stack_driver_position){
        fprintf(stderr, "error: basicblock content corrupted stack driver position\n");
    }

    // End if
    printf("[-]]");
}

void emit_stack_driver_push_all(){
    u32 local_cells_on_tape = emit_context.current_cell_index - emit_context.stack_driver_position;

    // stack[stack_pointer..] = local_cells[..]
    copy_cells_static(emit_context.stack_pointer, 4);
    // TODO: Make `move_cells_dynamic_uXX` follow conventions
    printf("<");
    emit_context.current_cell_index--;
    move_cells_dynamic_u32(emit_context.stack_begin, local_cells_on_tape);

    // Push number of local cells on to stack
    // push<u32>(stack, local_cells_on_tape)
    emit_u32(local_cells_on_tape);
    copy_cells_static(emit_context.stack_pointer, 4);
    printf("<");
    emit_context.current_cell_index--;
    move_cells_dynamic_u32(emit_context.stack_begin + local_cells_on_tape /* HACK: Offset without runtime computation */, 4);

    // stack_pointer = `stack_pointer + local_cells_on_tape + 4`
    emit_u32(local_cells_on_tape + 4);
    copy_cells_dynamic_u32(emit_context.stack_pointer, 4);
    emit_additive_u32(true);
    printf("<");
    emit_context.current_cell_index--;
    move_cells_static(emit_context.stack_pointer, 4);
}

void emit_stack_driver_pop_all(){
    // Compute `stack_pointer - 4`
    copy_cells_static(emit_context.stack_pointer, 4);
    emit_u32(4);
    emit_additive_u32(false);

    // Get number of local cells
    copy_cells_dynamic_u32(emit_context.stack_begin, 4);

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

    // Move bytes onto stack
    emit_stack_pointer();
    printf("<");
    emit_context.current_cell_index--;
    move_cells_dynamic_u32(emit_context.stack_begin, num_cells);

    // Increase stack pointer
    emit_stack_pointer();
    emit_u32(num_cells);
    emit_additive_u32(true);
    emit_set_stack_pointer();
}

void emit_stack_pop_n(u32 num_cells){
    // Compute `stack_pointer - n`
    emit_stack_pointer();
    emit_u32(num_cells);
    emit_additive_u32(false);

    // Set stack pointer to `stack_pointer - num_cells`
    dupe_cells(4);
    emit_set_stack_pointer();

    // Read cells from stack
    copy_cells_dynamic_u32(emit_context.stack_begin, num_cells);
}

void emit_recursive_functions(){
    for(u32 i = 0; i < num_functions; i++){
        Function function = functions[i];

        if(function.is_recursive){
            fprintf(stderr, "Building %d\n", (int) i);
            printf("\n");
        }
    }
}

