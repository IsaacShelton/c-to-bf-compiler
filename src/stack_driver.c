
#include <stdio.h>
#include "../include/emit.h"
#include "../include/emit_context.h"
#include "../include/storage.h"

void emit_stack_driver_pre(){
    // Initialize stack pointer to 4
    emit_u32(4);
    move_cells_static(emit_context.stack_pointer, 4);

    // Add entry point to stack
    emit_u32(1);
    move_cells_static(emit_context.stack_begin, 4);

    // Do while
    printf("[-]+[");

    // Get (stack_pointer - 4)
    copy_cells_static(emit_context.stack_pointer, 4);
    emit_u32(4);
    emit_additive_u32(false);

    // Set new stack pointer as stack pointer
    dupe_cells(4);
    move_cells_static(emit_context.stack_pointer, 4);

    // Get top entry (which is a basicblock id) from stack
    copy_cells_dynamic_u32(emit_context.stack_begin, 4);

    // Switch basicblock id
    emit_context.stack_driver_position = emit_context.current_cell_index;

    u32 space = aux_cstr_alloc((u8*) " ");
    u32 newline = aux_cstr_alloc((u8*) "\n");

    dupe_cells(4);
    emit_printu8();
    emit_print_aux_cstr(space);
    emit_printu8();
    emit_print_aux_cstr(space);
    emit_printu8();
    emit_print_aux_cstr(space);
    emit_printu8();
    emit_print_aux_cstr(newline);
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

