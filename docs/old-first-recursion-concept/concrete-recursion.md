
(global variables size)>      // user global variables
(stack size)>                 // stack variable
3>++++>                       // stack_pointer variable (starts at 4, since basicblock 0 aka the exit function is the return address)
4>                            // frame_pointer variable
3>+>                          // basicblock_pointer variable (entry point is basicblock 1)
(undefined)>                  // miscellaneous
============================= // working memory start

do {
    if(basicblock_pointer == 1){
        
    }

    if(basicblock_pointer == 2){

    }

    if(basicblock_pointer == 3){

    }

    if(basicblock_pointer == 4){

    }
    
    stack_pointer -= 4;
    basicblock_pointer = stack[stack_pointer];
} while(basicblock_pointer != 0);





// How to push n cells, where n is known at compile time

let size = emit_context.current_cell_index - working_memory_start; ===OR=== other compile-time known value

copy_cells_static(stack_pointer, 4); // Index

// Increment stack pointer
dupe(4);
emit_u32(size);
emit_additive_u32(true);

static_if(enable_stack_overflow_detection){
    dupe(4);
    emit_u32(stack_size);
    emit_gt_u32();
    printf("<"):
    [
        emit_print("STACK-OVERFLOW")
        emit_panicloop();
    ]
}

move_cells_static(stack_pointer);
printf("<"); // Because of weird move_cells_dynamic calling syntax
move_cells_dynamic_u32(stack /* static offset */, size);

// How to pop n cells from the stack, where n is known at compile time

copy_cells_static(stack_pointer, 4);
emit_u32(N);
emit_additive_u32(false);
dupe(4);
move_cells_static(stack_pointer, 4);
copy_cells_dynamic_u32(stack, N);

// How to read n cells from the stack frame, where n is known at compile time

copy_cells_static(frame_pointer, 4);
emit_u32(offset);
emit_additive_u32(true);
copy_cells_dynamic_u32(stack, N);

// How to write n cells to the stack frame, where n is known at compile time

emit_n_bytes();
copy_cells_static(frame_pointer, 4);
emit_u32(offset);
emit_additive_u32(true);
printf("<"); // Because of weird move_cells_dynamic calling syntax
move_cells_dynamic_u32(stack, N);

