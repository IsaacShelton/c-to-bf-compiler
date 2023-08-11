
#include <signal.h>
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

    if(destructive && size > 1){
        printf("%d<", size - 1);
        emit_context.current_cell_index -= size - 1;
    }
}

u0 move_cell_dynamic_u8(u32 destination_start_index){
    if(destination_start_index >= emit_context.current_cell_index){
        printf("\nwarning: move_cell_dynamic_u8 failed, can only move backwards\n");
        return;
    }

    u32 back_offset = emit_context.current_cell_index - destination_start_index - 1;

    // new_cell index
    //            ^

    // index
    //   ^

    /* Example:
        >>>>>> # Memory (array of 6 cells)
        ++++++++++> # Value (value that we want to write to the array)
        +++++ # Index (0 is leftmost and 5 is rightmost)
        >[-]>[-]>[-]<<< # Initialize moving window memory
        [>+>+>+<<<-] # Create three copies of index
        >>>[<<<+>>>-]<<< # Move third copy of index back to original location
        [ # While non zero index
        - # Decrement index
            # Move three index values forward
            >>>[-] # Zero next cell
            <[>+<-] # Move third cell to right
            <[>+<-] # Move second cell to right
            <[>+<-] # Move first cell to right
            <[>+<-] # Move value cell to right
            >>
        ]
        < # Point to value
        # Move value to destination
        <<<<<<[-]>>>>>>
        [<<<<<<+>>>>>>-]
        # Go back to where index was
        # Move index copy left
        >>>[<<+>>-]<<
        # Go left that amount
        [
            - # Decrement
            [<+>-] # Copy left
            >
            [<+>-] # Copy left index retainer cell
            << # Point to new index
        ]
        > # Point to offset retained index cell
        [<<+>>-] # Copy to replace cell that was moved
        << # Point to retained index cell
    */

    printf(">[-]>[-]>[-]3<"); // Initialize moving window memory

    printf("[>+>+>+<<<-]"); // Create three copies of index

    printf(">>>[<<<+>>>-]<<<"); // Move third copy of index back to original location

    printf("["); // While non zero index
        printf("-"); // Decrement index
        // Move three index values forward
        printf(">>>[-]"); // Zero next cell
        printf("<[>+<-]"); // Move third cell to right
        printf("<[>+<-]"); // Move second cell to right
        printf("<[>+<-]"); // Move first cell to right
        printf("<[>+<-]"); // Move value cell to right
        printf(">>");
    printf("]");

    printf("<"); // Point to value
    printf("%d<[-]%d>", back_offset, back_offset); // Move value to destination
    printf("[%d<+%d>-]", back_offset, back_offset); // Go back to where index was
    printf(">>>[<<+>>-]<<"); // Move index copy left

    // Go left that amount
    printf("[");
        printf("-"); // Decrement
        printf("[<+>-]"); // Copy left
        printf(">");
        printf("[<+>-]"); // Copy left index retainer cell
        printf("<<"); // Point to new index
    printf("]");

    printf(">"); // Point to offset retained index cell
    printf("[<<+>>-]"); // Copy to replace cell that was moved
    printf("<<"); // Point to retained index cell

    emit_context.current_cell_index--;
}

u0 move_cells_dynamic_u8(u32 destination_index, u32 size){
    if(size != 1){
        printf("\nwarning: can only use move_cells_dynamic_u8 with single cell for now\n");
    }

    move_cell_dynamic_u8(destination_index);
}

u0 emit_printu8(){
    // a ?
    //   ^

    // Clear 9 cells
    printf("[-]>[-]>[-]>[-]>[-]>[-]>[-]>[-]>[-]8<");
    
    // Print algorithm
    printf(">10+<<[->+>-[>+>>]>[+[-<+>]>+>>]6<]>>[-]3>10+<[->-[>+>>]>[+[-<+>]>+>>]5<]>[-]>>[>6+[-<8+>]<.<<+>+>[-]]<[<[->-<]6+[->8+<]>.[-]]<<6+[-<8+>]<.[-]<<[-<+>]");

    // Remain pointing to value cell
    printf(">");
}

u0 emit_additive_u8(u1 is_plus){
    // a b ?
    //     ^

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

u0 emit_divmod_u8(){
    // n d ?
    //     ^

    // 0 d-n%d n%d n/d
    // ^

    // Setup
    printf("[-]>[-]>[-]>[-]5<");

    // Divmod algorithm
    printf("[->[->+>>]>[<<+>>[-<+>]>+>>]5<]>[3>]>[[-<+>]>+>>]5<");

    // Consume 'd'
    emit_context.current_cell_index--;
}

u0 emit_divide_u8(){
    // a b ?
    //     ^

    emit_divmod_u8();

    // Go to n/d cell and move n/d to result cell
    printf("3>[3<+3>-]2<");
}

u0 emit_mod_u8(){
    // a b ?
    //     ^

    emit_divmod_u8();

    // Go to n%d cell and move n%d to result cell
    printf("2>[2<+2>-]<");
}

