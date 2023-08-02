
#include <stdio.h>
#include "../include/emit.h"
#include "../include/storage.h"
#include "../include/emit_context.h"

u0 set_cell_to_value(u8 new_value, u8 old_value){
    if(new_value == old_value){
        // nothing to do
    } else if(new_value > old_value){
        // increase by difference
        printf("%d+", new_value - old_value);
    } else {
        // decrease by difference
        printf("%d-", old_value - new_value);
    }
}

u0 emit_print_aux_cstr(u32 string){
    printf("[-]"); // zero character cell
    
    u8 previous_value = 0;

    for(u32 i = string; aux[i]; i++){
        set_cell_to_value(aux[i], previous_value);
        printf(".");
        previous_value = aux[i];
    }
}

u0 emit_print_array(u32 max_length){
    printf("%d<", max_length);

    for(u32 i = 0; i < max_length; i++){
        printf("[.>");
    }

    for(u32 i = 0; i < max_length; i++){
        printf("<[-]]");
    }

    emit_context.current_cell_index -= max_length;
}

u0 copy_cell_static(u32 start_index){
    u32 current_cell_index = emit_context.current_cell_index;
    if(start_index >= current_cell_index) return;

    u32 offset = (current_cell_index - start_index);

    // Create two empty cells, one for result, one for retaining
    printf("[-]>[-]<");

    // Go back offset cells
    printf("%d<", offset);

    // While 'from' cell is non zero,
    printf("[");

    // Go forward to result and retain cells
    printf("%d>", offset);

    // Increment result and retain cells
    printf("+>+");
    
    // Go back offset to original cell
    printf("%d<", offset + 1);

    // Decrement original cell
    printf("-");

    // End while
    printf("]");

    // Go forward to result and retain cells
    printf("%d>", offset);

    // Move 'retain' cell back to original cell
    // --------------------------------------------

    // Point to 'retain' cell
    printf(">");

    // While 'retain' cell is non zero,
    printf("[");

    // Go back (offset + 1) cells
    printf("%d<", offset + 1);

    // Increment original cell
    printf("+");

    // Go forward to retain cell
    printf("%d>", (offset + 1));

    // Decrement retain cell
    printf("-");

    // End while
    printf("]");

    // Increment current cell index
    emit_context.current_cell_index += 1;
}

u0 copy_cells_static(u32 start_index, u32 size){
    for(u32 i = 0; i < size; i++){
        copy_cell_static(start_index);
    }
}

u0 move_cell_static(u32 destination_index){
    u32 current_cell_index = emit_context.current_cell_index;
    if(destination_index >= current_cell_index) return;

    u32 offset = (current_cell_index - destination_index);

    // Zero destination cell
    printf("%d<", offset);
    printf("[-]");
    printf("%d>", offset);

    // While source cell is non-zero
    printf("[");

    // Go backwards to destination
    printf("%d<", offset);

    // Increment destination cell
    printf("+");

    // Go forwards to source
    printf("%d>", offset);

    // Decrement source cell
    printf("-");

    // End while
    printf("]");
}

u0 move_cells_static(u32 destination_index, u32 size, u1 destructive){
    for(u32 i = 0; i < size; i++){
        // Copy cell to destination
        move_cell_static(destination_index + size - i - 1);
    }

    if(destructive && size > 0){
        printf("%d<", size - 1);
        emit_context.current_cell_index -= size - 1;
    }
}

u0 emit_additive_u8(u1 is_plus){
    // Point to b
    printf("<");

    // While b is non-zero
    printf("[");

    // Go backwards to a
    printf("<");

    if(is_plus){
        // Increment a
        printf("+");
    } else {
        // Decrement a
        printf("-");
    }

    // Go forwards to b
    printf(">");

    // Decrement b
    printf("-");

    // End while
    printf("]");

    emit_context.current_cell_index--;
}

