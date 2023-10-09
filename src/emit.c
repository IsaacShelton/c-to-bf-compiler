
#include <signal.h>
#include <stdio.h>
#include "../include/emit.h"
#include "../include/storage.h"
#include "../include/emit_context.h"
#include "../include/function_emit.h"

u0 dupe_cell(){
    // a ?
    //   ^

    // Zero next two cells
    printf("[-]>[-]<<");

    // Dupe rightward
    printf("[>+>+<<-]");

    // Move second copy back into original
    printf(">>[<<+>>-]");

    // Remain pointing to next available cell
    // (nothing to do)

    emit_context.current_cell_index++;
}

u0 dupe_cell_offset(u32 offset){
    // a ?
    //   ^

    // Zero next two cells, and go to target read cell
    printf("[-]>[-]%d<", offset + 1);

    // Dupe into next available cell
    printf("[%d>+>+%d<-]", offset, offset + 1);

    // Move second copy back into original
    printf("%d>[%d<+%d>-]", offset + 1, offset + 1, offset + 1);

    // Remain pointing to next available cell
    // (nothing to do)

    emit_context.current_cell_index++;
}

u0 dupe_cells(u32 amount){
    for(u32 i = 0; i < amount; i++){
        dupe_cell_offset(amount);
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

    emit_context.current_cell_index++;
}

u0 copy_cells_static(u32 start_index, u32 size){
    for(u32 i = 0; i < size; i++){
        copy_cell_static(start_index + i);
    }
}

u0 emit_swap_u8(){
    // a b
    //     ^

    // Zero temporary cell
    printf("[-]");

    // Go to 'b' cell
    printf("<");

    // Move 'b' to temporary cell
    printf("[->+<]");

    // a 0 b
    //   ^
    
    // Go to 'a' cell
    printf("<");

    // Move 'a' cell to 'b' cell
    printf("[>+<-]");

    // Go to temporary cell
    printf(">>");

    // Move temporary cell to 'a' cell
    printf("[<<+>>-]");

    // Remain pointing at next available cell
}

u0 copy_cell_dynamic_u8_maintain(u32 start_index){
    // u8_dynamic_index
    //                  ^

    // value1 u8_dynamic_index 
    //                         ^

    dupe_cell();
    copy_cell_dynamic_u8(start_index);
    emit_swap_u8();
}

u0 copy_cell_dynamic_u16_maintain(u32 start_index){
    // minor_index major_index
    //                         ^

    // value1 minor_index major_index 
    //                                ^

    dupe_cells(2);
    copy_cell_dynamic_u16(start_index);

    // minor_index major_index value1
    //                                ^

    // Move value to be before duplicated index
    printf("[-]<[>+<-]<[>+<-]<[>+<-]3>[3<+3>-]");

    // value1 minor_index major_index 
    //                                ^
}

u0 copy_cells_dynamic_u8(u32 start_index, u32 size){
    // u8_dynamic_index
    //                  ^

    // value1 value2 value3 valueN
    //                             ^

    if(size == 0) return;

    for(u32 i = 0; i < size - 1; i++){
        copy_cell_dynamic_u8_maintain(start_index + i);
    }

    copy_cell_dynamic_u8(start_index + size - 1);
}

u0 copy_cells_dynamic_u16(u32 start_index, u32 size){
    // minor_index major_index
    //                         ^

    // value1 value2 value3 valueN
    //                             ^

    if(size == 0) return;

    for(u32 i = 0; i < size - 1; i++){
        copy_cell_dynamic_u16_maintain(start_index + i);
    }

    copy_cell_dynamic_u16(start_index + size - 1);
}

u0 copy_cell_dynamic_u8(u32 start_index){
    if(start_index >= emit_context.current_cell_index){
        printf("\nwarning: copy_cell_dynamic_u8 failed, can only copy backwards\n");
        return;
    }

    // index
    //        ^

    // value
    //        ^

    // Go to 'index' cell
    printf("<");
    emit_context.current_cell_index--;

    u32 back_offset = emit_context.current_cell_index - start_index;

    /* Example:
        ++++++++++ +>
        ++++++++++ ++>
        ++++++++++ +++>
        ++++++++++ ++++>
        ++++++++++ +++++>
        ++++++++++ ++++++> # Memory (array of 6 cells)

        ++++ # Index (0 is leftmost and 5 is rightmost)

        # Corrupt forward memory
        >+>+>+>+>+>+>+>+>+
        <<<<<<<<<

        >[-]>[-]<< # Initialize moving window memory

        [>+>+<<-] # Create two copies of index
        >>[<<+>>-]<<

        [
          - # Decrement index
          # Move two index values forward
          >>[-] # Zero next cell
          <[>+<-] # Move second cell to right
          <[>+<-] # Move first cell to right
          >
        ]

        # Copy
        [-]>>[-]<<
        <<<<<<
        [>>>>>> + >>+<<  <<<<<<-]
        >>>>>>

        # Move second copy back into original
        >>
        [<<<<<< <<+>> >>>>>>-]

        # Go back to where index was
        <

        # While index
        [
          - # Decrement
          <
          [<+>-] # Copy left
          >
          [<+>-] # Copy left
          <
        ]
    */


    printf(">[-]>[-]<<"); // Initialize moving window memory

    printf("[>+>+<<-]"); // Create two copies of index
    printf(">>[<<+>>-]<<");

    printf("[");
      printf("-"); // Decrement index
      // Move two index values forward
      printf(">>[-]"); // Zero next cell
      printf("<[>+<-]"); // Move second cell to right
      printf("<[>+<-]"); // Move first cell to right
      printf(">");
    printf("]");

    // Copy
    printf("[-]>>[-]<<");
    printf("%d<", back_offset);
    printf("[%d>+>>+<<%d<-]", back_offset, back_offset);
    printf("%d>", back_offset);

    // Move second copy back into original
    printf(">>");
    printf("[%d<<<+>>%d>-]", back_offset, back_offset);

    // Go back to where index was
    printf("<");

    // While index
    printf("[");
      printf("-"); // Decrement
      printf("<");
      printf("[<+>-]"); // Copy left
      printf(">");
      printf("[<+>-]"); // Copy left
      printf("<");
    printf("]");

    emit_context.current_cell_index++;
}

u0 copy_cell_dynamic_u16(u32 start_index){
    if(start_index >= emit_context.current_cell_index){
        printf("\nwarning: copy_cell_dynamic_u16 failed, can only copy backwards\n");
        return;
    }

    // minor_index major_index
    //                         ^

    // value
    //       ^
    
    /*
    EXAMPLE:
        # Memory (array of 20 cells)
		++++++++++ +>
        ++++++++++ ++>
        ++++++++++ +++>
        ++++++++++ ++++>
        ++++++++++ +++++>
        ++++++++++ ++++++>
        ++++++++++ +++++++>
        ++++++++++ ++++++++>
        ++++++++++ +++++++++>
        ++++++++++ ++++++++++>
        ++++++++++ ++++++++++ +>
        ++++++++++ ++++++++++ ++>
        ++++++++++ ++++++++++ +++>
        ++++++++++ ++++++++++ ++++>
        ++++++++++ ++++++++++ +++++>
        ++++++++++ ++++++++++ ++++++>
        ++++++++++ ++++++++++ +++++++>
        ++++++++++ ++++++++++ ++++++++>
        ++++++++++ ++++++++++ +++++++++>
        ++++++++++ ++++++++++ ++++++++++>


		# Minor index
        [-]++++>
        # Major index
        [-]+>
        <

        # Corrupt forward memory
        >+>+>+>+>+>+>+>+>+
        >+>+>+>+>+>+>+>+>+
        >+>+>+>+>+>+>+>+>+
        >+>+>+>+>+>+>+>+>+
        <<<<<<<<<
        <<<<<<<<<
        <<<<<<<<<
        <<<<<<<<<
        
        # Create copy of index
        >[-]>[-]>[-]>[-]
        <<<<
        [>>+>>+<<<<-]
        <
        [>>+>>+<<<<-]
        >>>>
        [<<<<+>>>>-]
        >
        [<<<<+>>>>-]
        
        # Go to major index of first index pair
        <<<<
        
        [
        	-
            
            # Go to minor index of first pair
            <
            
            # Copy forward
            >>>>>>>>>>
            [-]>[-]>[-]>[-]>
            <<<<<<<<<<
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            >
            >>>>>>>>>>
        ]
        
        # Go to minor index
        <
        
        # Go forward by minor index
        [
        	-
            >>>>[-]
            <[>+<-]
            <[>+<-]
            <[>+<-]
            <[>+<-]
            >
        ]
        
        # Move data to make two copies
        <<<<<<<<<<
        <<<<<<<<<<
        [
        	>>>>>>>>>>
            >>>>>>>>>>
            +>+<
            <<<<<<<<<<
            <<<<<<<<<<
            -
        ]
        >>>>>>>>>>
        >>>>>>>>>>
        
        >
        
        # Move second data copy back
        [
        	<
        	<<<<<<<<<<
            <<<<<<<<<<
            +
            >>>>>>>>>>
            >>>>>>>>>>
            >
            -
        ]
        
        # Go to minor index
        >
        
        # Go backwards by minor index
        [
        	-
            <<
            [<+>-]>
            >
            [<+>-]>
            [<+>-]
            <<
        ]
        
        # Go to major index
        >
        
        # Go backwards by 256 (10 in example) times minor index
        [
        	-
        	[<<<<<<<<<<+>>>>>>>>>>-]
            <<<
            [<<<<<<<<<<+>>>>>>>>>>-]
            >>>
            <<<<<<<<<<
        ]
        
        # Point to next cell
        <<
        */

    // Go to 'index' cell
    printf("<");
    emit_context.current_cell_index--;

    u32 back_offset = emit_context.current_cell_index - start_index - 1;

    // Create copy of index
    printf(">[-]>[-]>[-]>[-]");
    printf("<<<<");
    printf("[>>+>>+<<<<-]");
    printf("<");
    printf("[>>+>>+<<<<-]");
    printf(">>>>");
    printf("[<<<<+>>>>-]");
    printf(">");
    printf("[<<<<+>>>>-]");
    
    // Go to major index of first index pair
    printf("<<<<");
    
    printf("[");
        printf("-");
        
        // Go to minor index of first pair
        printf("<");
        
        // Copy forward
        printf("256>");
        printf("[-]>[-]>[-]>[-]>");
        printf("256<");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf(">");
        printf("256>");
    printf("]");
    
    // Go to minor index
    printf("<");
    
    // Go forward by minor index
    printf("[");
        printf("-");
        printf(">>>>[-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf(">");
    printf("]");
    
    // Move data to make two copies
    printf("%d<", back_offset);
    printf("[");
        printf("%d>", back_offset);
        printf("+>+<");
        printf("%d<", back_offset);
        printf("-");
    printf("]");
    printf("%d>", back_offset);
    
    printf(">");
    
    // Move second data copy back
    printf("[");
        printf("<");
        printf("%d<", back_offset);
        printf("+");
        printf("%d>", back_offset);
        printf(">");
        printf("-");
    printf("]");
    
    // Go to minor index
    printf(">");
    
    printf("[");
        printf("-");
        printf("<<");
        printf("[<+>-]>");
        printf(">");
        printf("[<+>-]>");
        printf("[<+>-]");
        printf("<<");
    printf("]");
    
    // Go to major index
    printf(">");
    
    // Go backwards by 256 times minor index
    printf("[");
        printf("-");
        printf("[256<+256>-]");
        printf("<<<");
        printf("[256<+256>-]");
        printf(">>>");
        printf("256<");
    printf("]");
    
    // Point to next cell
    printf("<<");

    // No further changes to `emit_context.current_cell_index`
}

u0 move_cell_static(u32 destination_index){
    u32 current_cell_index = emit_context.current_cell_index;
    u32 offset;
    u8 towards, backwards;

    if(destination_index <= current_cell_index){
        offset = current_cell_index - destination_index;
        towards = '<';
        backwards = '>';
    } else {
        offset = destination_index - current_cell_index;
        towards = '>';
        backwards = '<';
    }

    // Zero destination cell
    printf("%d%c", offset, towards);
    printf("[-]");
    printf("%d%c", offset, backwards);

    // While source cell is non-zero
    printf("[");

    // Go backwards to destination
    printf("%d%c", offset, towards);

    // Increment destination cell
    printf("+");

    // Go forwards to source
    printf("%d%c", offset, backwards);

    // Decrement source cell
    printf("-");

    // End while
    printf("]");
}

u0 move_cells_static(u32 destination_index, u32 size, u1 destructive){
    // data1 data2 data3 data4
    //                     ^

    // ?
    // ^

    for(u32 i = 0; i < size; i++){
        move_cell_static(destination_index + size - i - 1);

        if(i + 1 < size){
            printf("<");
            emit_context.current_cell_index--;
        }
    }
}

u0 move_cell_dynamic_u8(u32 destination_start_index){
    if(destination_start_index >= emit_context.current_cell_index){
        printf("\nwarning: move_cell_dynamic_u8 failed, can only move backwards\n");
        return;
    }

    u32 back_offset = emit_context.current_cell_index - destination_start_index - 1;

    // value index
    //         ^

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

u0 move_cell_dynamic_u16(u32 destination_start_index){
    if(destination_start_index >= emit_context.current_cell_index){
        printf("\nwarning: move_cell_dynamic_u16 failed, can only move backwards\n");
        return;
    }

    u32 back_offset = emit_context.current_cell_index - destination_start_index - 2;

    // value index_minor index_major
    //                        ^

    // index_minor index_major
    //      ^

    /* Example:
        # Garble memory
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        +>+>+>+>+>+>+>+>+>+>
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<

 		>>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>> # Memory (array of 50 cells)
        +++++++++++++> # Value (value that we want to write to the array)
        [-]+++++>[-]++++> # Index little endian
        
        # Initialize moving window memory
        [-]>[-]>[-]>[-]>[-]>[-]>[-]<<<<<<
        
        # Go to minor index
        << 
        
         # Create three copies of index
        [>>+>>+>>+<<<<<<-]
        >[>>+>>+>>+<<<<<<-]
        
        # Move data here
        <<[>>+<<-]
        
        # Move third copy here
        >>>>>>>>
        [<<<<<<<+>>>>>>>-]
        <
        [<<<<<<<+>>>>>>>-]
        
        # Point to major index
        <<<
        
        [ # While non zero major index
        - # Decrement index
            # Move forward
            <
            >>>>>>>>>>
            <
            [-]>[-]>[-]>[-]>[-]> # Zero destination
            >
            <
            <<<<<<<<<<
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            <[>>>>>>>>>>+<<<<<<<<<<-]
            >>>>>>>>>>
            >>
        ]
        
        # Go to minor cell
        <
       
        [ # While non zero major index
        - # Decrement index
            # Move three index values forward
            <
            >>>>>
            [-]> # Zero destination
            <
            <[>+<-]
            <[>+<-]
            <[>+<-]
            <[>+<-]
            <[>+<-]
            >
            >
        ]
        
        <
        
        # Move into place
        << # Account for duped index offset
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        [-]
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >> # Account for duped index offset
        [
        << # Account for duped index offset
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        <<<<<<<<<<
        +
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >>>>>>>>>>
        >> # Account for duped index offset
        -
        ]
        
        # Copy second copy of index to index position
        >>>
        [<<<+>>>-]
        >
        [<<<+>>>-]
        <<<
        
        # Go to minor index
        <
        
        [-
        	[<+>-]
            >
            [<+>-]
            <<
        ]
        
        # Go to major index
        >
        
        [-
        	[
            <<<<<<<<<<+>>>>>>>>>>-
            ]
            <<<<<<<<<<
        ]
        
        # To back to zeroed minor index
        <
        
        # Point to next available cell (left over duped index which can be used again)
        <<
    */

    // Initialize moving window memory
    printf(">[-]>[-]>[-]>[-]>[-]>[-]>[-]");
    printf("6<");
    
    // Go to minor index
    printf("<<");
    
    // Create three copies of index
    printf("[>>+>>+>>+6<-]");
    printf(">");
    printf("[>>+>>+>>+6<-]");
    
    // Go to data cell
    printf("<<");

    // Move data cell to third cell
    printf("[>>+<<-]");
    
    // Move third copy of index to first two cells
    printf("8>");
    printf("[7<+7>-]");
    printf("<");
    printf("[7<+7>-]");
    
    // Point to major index
    printf("3<");

    // While non zero major index
    printf("[");
        // Decrement index
        printf("-");

        // Move forward
        printf("<");
        printf("256>");
        printf("<");
        printf("[-]>[-]>[-]>[-]>[-]>"); // Zero destination
        printf("256<");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("<[256>+256<-]");
        printf("256>");
        printf(">>");
    printf("]");
    
    // Go to minor cell
    printf("<");
   
    // While non zero major index
    printf("[");

        // Decrement index
        printf("-");

        // Move three index values forward
        printf("4>");
        printf("[-]"); // Zero destination
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf("<[>+<-]");
        printf(">>");
    printf("]");
    
    // Go to data cell
    printf("<");
    
    // Move into place
    printf("%d<[-]%d>[%d<+%d>-]", 2 + back_offset, 2 + back_offset, 2 + back_offset, 2 + back_offset); // Account for duped index offset
    
    // Copy second copy of index to index position
    printf("3>");
    printf("[3<+3>-]");
    printf(">");
    printf("[3<+3>-]");
    printf("3<");
    
    // Go to minor index
    printf("<");
    
    // Go backwards by minor index
    printf("[");
        printf("-");
        printf("[<+>-]");
        printf(">");
        printf("[<+>-]");
        printf("<<");
    printf("]");
    
    // Go to major index
    printf(">");
    
    // Go backwards by 256 times major index
    printf("[");
        printf("-");
        printf("[256<+256>-]");
        printf("256<");
    printf("]");
    
    // To back to zeroed minor index
    printf("<");
    
    // Point to next available cell (left over duped index which can be used again)
    printf("<<");

    emit_context.current_cell_index -= 2;
}

u0 move_cells_dynamic_u8(u32 destination_index, u32 size){
    // value1 value2 value3 index
    //                        ^

    // index
    //   ^

    for(u32 i = 0; i < size; i++){
        move_cell_dynamic_u8(destination_index + size - 1 - i);
    }
}

u0 move_cells_dynamic_u16(u32 destination_index, u32 size){
    // value1 value2 value3 minor_index major_index
    //                                       ^

    // minor_index major_index
    //      ^

    for(u32 i = 0; i < size; i++){
        move_cell_dynamic_u16(destination_index + size - 1 - i);

        if(i + 1 != size){
            printf(">");
            emit_context.current_cell_index++;
        }
    }
}

u0 print_cells_static(u32 start_index, u32 max_length){
    u32 current_cell_index = emit_context.current_cell_index;

    if(start_index >= current_cell_index){
        fprintf(stderr, "warning: print_cells_static cannot print cells that are in front of current cell index\n");
        return;
    }

    u32 back_offset = (current_cell_index - start_index);

    /* EXAMPLE:
        ++++++++++ ++++++++++ ++++++++++
        ++++++++++ ++++++++++ ++++++++++ 
        ++++++++++ >
        ++++++++++ ++++++++++ ++++++++++
        ++++++++++ ++++++++++ ++++++++++ 
        ++++++++++ +>
        ++++++++++ ++++++++++ ++++++++++
        ++++++++++ ++++++++++ ++++++++++ 
        ++++++++++ ++>
        ++++++++++ ++++++++++ ++++++++++
        ++++++++++ ++++++++++ ++++++++++ 
        ++++++++++ +++>

        +++++> # Example value being used by something else

        +> # Have at least one cell between referenced value
             # and ourselves so that there can be a cell
             # that we will temporarily set to zero

        # Go to first character
        <<<< <<

        [>>>> >>+<< <<<<-]>>>> >> # Move first character to first slot
        >
        <<< <<<< >>>> # Move trailing to second slot
        [
            >>> >>>> <<<< +
            <<< <<<< >>>> -
        ]
        >>> >>>> <<<<

        <

        # Print
        [
        . # Print first character

        # Print insides
        < <<<<
        [.>]<[<]
        > >>>>>

        # Move first character back
        [<<<< << + >> >>>> -]
        ]

        # Go to second slot
        >
        # Move trailing character back
        [<<< <<<< >>>> + >>> >>>> <<<< -]

        # Point to next available cell
        <<
    */

    // Have at least one cell between referenced value
    // and ourselves so that there can be a cell
    // that we will temporarily zero

    // Allocate two cells
    printf("[-]>[-]<");

    // Go to first character
    printf("%d<", back_offset);

    // Move first character to first slot
    printf("[%d>+%d<-]", back_offset + 1, back_offset + 1);

    // Go to trailing cell
    printf("%d>", max_length);

    // Move trailing character to second slot
    printf("[%d>+%d<-]", back_offset - max_length + 2, back_offset - max_length + 2);

    // Go to first slot
    printf("%d>", back_offset - max_length + 1);

    // Print string if first slot is non-zero
    printf("[");
        printf("."); // Print first character

        // Print insides
        printf("%d<", back_offset);
        printf("[.>]<[<]");
        printf("%d>", back_offset + 1);

        // Move first character back
        printf("[%d<+%d>-]", back_offset + 1, back_offset + 1);
    printf("]");

    // Go to second slot
    printf(">");

    // Move trailing character back
    printf("[%d<+%d>-]", back_offset - max_length + 2, back_offset - max_length + 2);

    // Point to next available cell
    printf("<<");
}

u0 emit_print_array_value(u32 max_length){
    print_cells_static(emit_context.current_cell_index - max_length, max_length);
    printf("%d<", max_length);
    emit_context.current_cell_index -= max_length;
}

u0 emit_u1(u1 value){
    printf("[-]");

    if(value){
        printf("+");
    }

    printf(">");
    emit_context.current_cell_index++;
}

u0 emit_u8(u8 value){
    printf("[-]%d+>", value);
    emit_context.current_cell_index++;
}

u0 emit_u16(u16 value){
    // Little endian
    emit_u8(value & 0xFF);
    emit_u8((value >> 8) & 0xFF);
}

u0 emit_u24(u24 value){
    /*
    emit_u8(value >> 16);
    emit_u8((value >> 8) & 0xFF);
    emit_u8(value & 0xFF);
    */

    // Little endian
    emit_u8(value.bytes[2]);
    emit_u8(value.bytes[1]);
    emit_u8(value.bytes[0]);
}

u0 emit_u32(u32 value){
    // Little endian
    emit_u8(value & 0xFF);
    emit_u8((value >> 8) & 0xFF);
    emit_u8((value >> 16) & 0xFF);
    emit_u8(value >> 24);
}

u0 emit_printu1(){
    // a ?
    //   ^

    // Allocate 3 temporary cells ('whether to run else', 'copied value', and 'bus')
    // and set 'whether to run else' as true
    printf("[-]+>[-]>[-]");

    // Go to value
    printf("3<");

    // Copy value from 'a' to 'copied value' via 'bus'
    printf("[>>+>+<<<-]");
    printf("3>[3<+3>-]");

    // Go to 'copied value' cell
    printf("<");

    // If copied value
    printf("[");
        // Set 'whether to run else' to false
        printf("<->");

        // Print "true"
        printf("[-]116+.2-.3+.16-.");

        // Zero copied value
        printf("[-]");
    // End if
    printf("]");

    // Go to 'else' cell
    printf("<");
    
    // If 'else'
    printf("[");
        // Print "false"
        printf("[-]102+.5-.11+.7+.14-.");

        // Zero 'else'
        printf("[-]");
    // End if
    printf("]");

    // Remain pointing to next available cell
    // (nothing to do)
}

u0 emit_printu8(){
    // a ?
    //   ^

    // Clear 9 cells
    printf("[-]>[-]>[-]>[-]>[-]>[-]>[-]>[-]>[-]8<");
    
    // Print algorithm
    printf(">10+<<[->+>-[>+>>]>[+[-<+>]>+>>]6<]>>[-]3>10+<[->-[>+>>]>[+[-<+>]>+>>]5<]>[-]>>[>6+[-<8+>]<.<<+>+>[-]]<[<[->-<]6+[->8+<]>.[-]]<<6+[-<8+>]<.[-]<<[-<+>]");
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

u0 emit_stdlib_function_call(u8 function_name[16], u32 return_size, u32 params_size){
    // Make room for return value
    for(u32 i = 0; i < return_size; i++){
        printf("[-]>");
    }
    emit_context.current_cell_index += return_size;

    // Move parameters over to make room for return value
    printf("<");
    printf("%d<", return_size);

    for(u32 i = 0; i < params_size; i++){
        printf("[-%d>+%d<]", return_size, return_size);
        printf("<");
    }

    printf("%d>", 1 + return_size + params_size);

    u32 start_function_cell_index = emit_context.current_cell_index - params_size;

    u32 prev_num_aux = num_aux;
    u32 name = aux_cstr_alloc(function_name);
    u32 function_index = find_function(name);
    num_aux = prev_num_aux;
    
    if(function_index >= FUNCTIONS_CAPACITY){
        fprintf(stderr, "Failed to emit standard library function call, could not find function\n");
        return;
    }

    if(function_emit(function_index, start_function_cell_index, emit_context.current_cell_index)){
        fprintf(stderr, "Failed to emit standard library function call, errors exist inside of function\n");
        return;
    }
}

u0 emit_additive_u16(u1 is_plus){
    if(is_plus){
        emit_stdlib_function_call((u8*) "u16_add", 2, 4);
    } else {
        emit_stdlib_function_call((u8*) "u16_sub", 2, 4);
    }
}

u0 emit_additive_u32(u1 is_plus){
    if(is_plus){
        emit_stdlib_function_call((u8*) "u32_add", 4, 8);
    } else {
        emit_stdlib_function_call((u8*) "u32_sub", 4, 8);
    }
}

u0 emit_multiply_u8(){
    // a b ?
    //     ^

    // Multiply algorithm

    // Allocate two temporary cells
    printf("[-]>[-]");

    // Go to 'a' cell
    printf("3<");

    // Move 'a' cell to second temporary cell
    printf("[3>+3<-]");

    // Go to second temporary cell
    printf("3>");

    // While second temporary cell
    printf("[");

    // Point to 'b' cell
    printf("<<");

    // Add 'b' cell to 'a' cell and first temporary cell
    printf("[<+>>+<-]");

    // Go to first temporary cell
    printf(">");

    // Move to first temporary cell to 'b' cell
    printf("[<+>-]");

    // Go to second temporary cell and decrement it
    printf(">-");

    // End while
    printf("]");

    // Point to next available cell
    printf("<<");

    emit_context.current_cell_index--;
}

u0 emit_multiply_u16(){
    emit_stdlib_function_call((u8*) "u16_mul", 2, 4);
}

u0 emit_multiply_u32(){
    emit_stdlib_function_call((u8*) "u32_mul", 4, 8);
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
    emit_context.current_cell_index -= 2;
}

u0 emit_divide_u8(){
    // a b ?
    //     ^

    emit_divmod_u8();

    // Go to n/d cell and move n/d to result cell
    printf("3>[3<+3>-]2<");
    emit_context.current_cell_index++;
}

u0 emit_divide_u16(){
    emit_stdlib_function_call((u8*) "u16_div", 2, 4);
}

u0 emit_divide_u32(){
    emit_stdlib_function_call((u8*) "u32_div", 4, 8);
}

u0 emit_mod_u8(){
    // a b ?
    //     ^

    emit_divmod_u8();

    // Go to n%d cell and move n%d to result cell
    printf("2>[2<+2>-]<");
    emit_context.current_cell_index++;
}

u0 emit_mod_u16(){
    emit_stdlib_function_call((u8*) "u16_mod", 2, 4);
}

u0 emit_mod_u32(){
    emit_stdlib_function_call((u8*) "u32_mod", 4, 8);
}

u0 emit_lshift_u8(){
    // a b ?
    //     ^

    // Create temporary cell
    printf("[-]");

    // Go to 'b' cell
    printf("<");

    // While 'b'
    printf("[");

    // Go to 'a' cell
    printf("<");

    // Move 'a' cell to temporary cell
    printf("[>>+<<-]");

    // Go to temporary cell
    printf(">>");

    // Store 2 times the temporary cell in 'a' cell
    printf("[<<++>>-]");

    // Go to 'b' cell and decrement
    printf("<-");
    
    // End while
    printf("]");

    emit_context.current_cell_index--;
}

u0 emit_lshift_u16(){
    // Convert u16 amount to u8 amount
    printf("<");
    emit_context.current_cell_index--;

    emit_stdlib_function_call((u8*) "u16_sll", 2, 3);
}

u0 emit_lshift_u32(){
    // Convert u32 amount to u8 amount
    printf("3<");
    emit_context.current_cell_index -= 3;

    emit_stdlib_function_call((u8*) "u32_sll", 4, 5);
}

u0 emit_rshift_u8(){
    // a b ?
    //     ^

    // Allocate 4 temporary cells (c, x, y, and z)
    printf("[-]>[-]>[-]>[-]");

    // Go to 'b' cell
    printf("4<");

    // While 'b'
    printf("[");

    // Set 'c' cell to 2
    printf(">++");

    // Point to 'a' cell
    printf("<<");

    // While 'a'
    printf("[");
    
    // Decrement 'a' cell
    printf("-");

    // Go to and decrement 'c' cell
    printf(">>-");

    // Copy 'c' cell to 'y' cell via 'z' cell
    printf("[>>+>+3<-]3>[3<+3>-]");

    // Go to 'y' cell
    printf("<");

    // If 'y' cell is 0
    printf("-[");

    // Go to and increment 'x' cell
    printf("<+");

    // Set 'c' cell to 2
    printf("<++");

    // Go to and semi-zero 'y' cell
    printf(">>+");

    // End if
    printf("]");

    // Go to 'a' cell
    printf("4<");

    // End while
    printf("]");

    // Go to 'x' cell
    printf("3>");

    // Move 'x' cell to 'a' cell
    printf("[3<+3>-]");

    // Zero 'c' cell
    printf("<[-]");

    // Go to and decrement 'b' cell
    printf("<-");

    // End while
    printf("]");

    emit_context.current_cell_index--;
}

u0 emit_rshift_u16(){
    // Convert u16 amount to u8 amount
    printf("<");
    emit_context.current_cell_index--;

    emit_stdlib_function_call((u8*) "u16_srl", 2, 3);
}

u0 emit_rshift_u32(){
    // Convert u32 amount to u8 amount
    printf("3<");
    emit_context.current_cell_index -= 3;

    emit_stdlib_function_call((u8*) "u32_srl", 4, 5);
}

u0 emit_eq_u8(){
    // a b ?
    //     ^

    // Go to 'a' cell
    printf("<<");

    // Subtract 'a' cell from 'b' cell
    printf("[->-<]");

    // Set 'a' cell to 1
    printf("+");

    // Go to 'b' cell
    printf(">");

    // If 'b' cell
    printf("[");
    
    // Semi-zero 'a' cell
    printf("<->");

    // End if
    printf("[-]]");

    // Remain pointing to next available cell
    // (nothing to do)
    
    emit_context.current_cell_index--;
}

u0 emit_eq_u16(){
    emit_stdlib_function_call((u8*) "u16_eq", 1, 4);
}

u0 emit_eq_u32(){
    emit_stdlib_function_call((u8*) "u32_eq", 1, 8);
}

u0 emit_neq_u8(){
    // a b ?
    //     ^

    // Go to 'a' cell
    printf("<<");

    // Subtract 'a' cell from 'b' cell
    printf("[->-<]");

    // Go to 'b' cell
    printf(">");

    // If 'b' cell
    printf("[");

    // Set 'a' cell to 1
    printf("<+>");

    // End if
    printf("[-]]");

    emit_context.current_cell_index--;
}

u0 emit_neq_u16(){
    emit_stdlib_function_call((u8*) "u16_eq", 1, 4);
    emit_not_u1();
}

u0 emit_neq_u32(){
    emit_stdlib_function_call((u8*) "u32_eq", 1, 8);
    emit_not_u1();
}

u0 emit_lt_u8(){
    // a b ?
    //     ^

    // Allocate two temporary cells ('c' and 'd')
    printf("[-]>[-]");

    // Go to 'b' cell
    printf("<<");

    // While 'b' cell
    printf("[");

    // Set 'c' cell to zero
    printf(">[-]");

    // Go to 'a' cell
    printf("<<");

    // Copy 'a' cell to 'd' cell via 'c' cell
    printf("[>>+>+3<-]>>[<<+>>-]");

    // Set 'c' cell to 1
    printf("+");

    // Go to 'd' cell
    printf(">");

    // If 'd' cell
    printf("[");

    // Zero 'd' cell
    printf("[-]");

    // Go to and semi-zero 'c' cell
    printf("<-");

    // Go to and decrement 'a' cell
    printf("<<-");

    // Go to 'd' cell
    printf("3>");

    // End if
    printf("]");

    // Go to 'b' cell and decrement
    printf("<<-");

    // End while
    printf("]");

    // Move 'c' cell to 'a' cell
    printf("<[-]>>[<<+>>-]");

    // Go to next available cell
    printf("<");

    emit_context.current_cell_index--;
}

u0 emit_lt_u16(){
    emit_stdlib_function_call((u8*) "u16_lt", 1, 4);
}

u0 emit_lt_u32(){
    emit_stdlib_function_call((u8*) "u32_lt", 1, 8);
}

u0 emit_gt_u8(){
    // a b ?
    //     ^

    // Allocate two temporary cells ('c' and 'd')
    printf("[-]>[-]");
    
    // Go to 'a' cell
    printf("3<");

    // While 'a'
    printf("[");

    // Go to and zero 'c' cell
    printf(">>[-]");

    // Go to 'b'
    printf("<");

    // Copy 'b' cell to 'd' cell via 'c' cell
    printf("[>+>+<<-]>[<+>-]");

    // Set 'c' cell to 1
    printf("+");

    // Go to 'd' cell
    printf(">");

    // If 'd' cell
    printf("[");

    // Zero 'd' cell
    printf("[-]");

    // Go 'c' cell and semi-zero
    printf("<-");

    // Go to 'b' cell and decrement
    printf("<-");

    // Go to 'd' cell
    printf(">>");

    // End if
    printf("]");

    // Go to 'a' cell
    printf("3<");

    // Decrement 'a' cell
    printf("-");

    // End while
    printf("]");

    // Move 'c' cell to 'a' cell
    printf(">>[<<+>>-]");
    
    // Go to next available cell
    printf("<");

    emit_context.current_cell_index--;
}

u0 emit_gt_u16(){
    emit_stdlib_function_call((u8*) "u16_gt", 1, 4);
}

u0 emit_gt_u32(){
    emit_stdlib_function_call((u8*) "u32_gt", 1, 8);
}

u0 emit_lte_u8(){
    // a b ?
    //     ^

    // Allocate two temporary cells ('c' and 'd'), with 'c' starting at 1
    printf("[-]+>[-]");

    // Go to 'a' cell
    printf("3<");

    // While 'a' cell
    printf("[");

    // Set 'c' cell to zero
    printf(">>[-]");

    // Go to 'b' cell
    printf("<");

    // Copy 'b' cell to 'd' cell via 'c' cell
    printf("[>+>+<<-]>[<+>-]");

    // Go to 'd' cell
    printf(">");

    // If 'd' cell
    printf("[");

    // Zero 'd' cell
    printf("[-]");

    // Go to 'c' cell and set to 1
    printf("<+");

    // Go to and decrement 'b' cell
    printf("<-");

    // Go to 'd' cell
    printf(">>");

    // End if
    printf("]");

    // Go to 'a' cell and decrement
    printf("3<-");

    // End while
    printf("]");

    // Move 'c' cell to 'a' cell
    printf(">>[<<+>>-]");

    // Go to next available cell
    printf("<");

    emit_context.current_cell_index--;
}

u0 emit_lte_u16(){
    emit_stdlib_function_call((u8*) "u16_lte", 1, 4);
}

u0 emit_lte_u32(){
    emit_stdlib_function_call((u8*) "u32_lte", 1, 8);
}

u0 emit_gte_u8(){
    // a b ?
    //     ^

    // Allocate two temporary cells ('c' and 'd'), with 'c' starting at 1
    printf("[-]+>[-]");
    
    // Go to 'b' cell
    printf("<<");

    // While 'b'
    printf("[");

    // Go to and zero 'c' cell
    printf(">[-]");

    // Go to 'a'
    printf("<<");

    // Copy 'a' cell to 'd' cell via 'c' cell
    printf("[>>+>+<<<-]>>[<<+>>-]");

    // Go to 'd' cell
    printf(">");

    // If 'd' cell
    printf("[");

    // Zero 'd' cell
    printf("[-]");

    // Go 'c' cell and set to 1
    printf("<+");

    // Go to 'a' cell and decrement
    printf("<<-");

    // Go to 'd' cell
    printf("3>");

    // End if
    printf("]");

    // Go to 'b' cell
    printf("<<");

    // Decrement 'b' cell
    printf("-");

    // End while
    printf("]");

    // Move 'c' cell to 'a' cell
    printf("<[-]>>[<<+>>-]");
    
    // Go to next available cell
    printf("<");

    emit_context.current_cell_index--;
}

u0 emit_gte_u16(){
    emit_stdlib_function_call((u8*) "u16_gte", 1, 4);
}

u0 emit_gte_u32(){
    emit_stdlib_function_call((u8*) "u32_gte", 1, 8);
}

static u0 emit_binary_bitwise_operator_pre(){
    // a b ?
    //     ^

    printf("5>");
    printf("[-]>>[-]<");
    printf("[-]8-[8+");
    printf("<");
    printf("<[-]<[-]<[-]<[-]<[-]");
    printf("++");
    printf("<<");
    printf("[");
    printf("-");
    printf(">>-");
    printf("[>+>>+3<-]>[<+>-]");
    printf(">>");
    printf(">>+<<");
    printf("-[");
    printf("<+");
    printf("<<++");
    printf("5>");
    printf("--");
    printf("<<");
    printf("+");
    printf("]");
    printf("5<");
    printf("]");
    printf("4>[4<+4>-]");
    printf("<<[-]++");
    printf("<");
    printf("[");
    printf("-");
    printf(">-");
    printf("[>+>>+3<-]>[<+>-]");
    printf(">>");
    printf(">+<");
    printf("-[");
    printf(">--<");
    printf("<+");
    printf("<<++");
    printf("3>");
    printf("+");
    printf("]");
    printf("4<");
    printf("]");
    printf("3>[3<+3>-]");
    printf(">>");
}

static u0 emit_binary_bitwise_operator_post(){
    printf(">[<+<+>>-]<[>+<-]");
    printf("<");
    printf("[");
    printf("<");
    printf("[<+>-]");
    printf("<[>++<-]");
    printf(">>-");
    printf("]");
    printf("<");
    printf("[4>+4<-]");
    printf("3>");
    printf("7-");
    printf("]");
    printf(">[9<+9>-]");
    printf("8<");
    emit_context.current_cell_index--;
}

u0 emit_bit_and_u8(){
    // a b ?
    //     ^

    emit_binary_bitwise_operator_pre();
    printf("[->[-<<+>>]<]>[-]");
    emit_binary_bitwise_operator_post();
}

u0 emit_bit_and_u16(){
    emit_stdlib_function_call((u8*) "u16_bit_and", 2, 4);
}

u0 emit_bit_and_u32(){
    emit_stdlib_function_call((u8*) "u32_bit_and", 4, 8);
}

u0 emit_bit_or_u8(){
    // a b ?
    //     ^

    emit_binary_bitwise_operator_pre();
    printf("[>+<-]>[[-]<<+>>]");
    emit_binary_bitwise_operator_post();
}

u0 emit_bit_or_u16(){
    emit_stdlib_function_call((u8*) "u16_bit_or", 2, 4);
}

u0 emit_bit_or_u32(){
    emit_stdlib_function_call((u8*) "u32_bit_or", 4, 8);
}

u0 emit_bit_xor_u8(){
    // a b ?
    //     ^

    emit_binary_bitwise_operator_pre();
    printf("[>-<-]>[[-]<<+>>]");
    emit_binary_bitwise_operator_post();
}

u0 emit_bit_xor_u16(){
    emit_stdlib_function_call((u8*) "u16_bit_xor", 2, 4);
}

u0 emit_bit_xor_u32(){
    emit_stdlib_function_call((u8*) "u32_bit_xor", 4, 8);
}

u0 emit_not_u1(){
    // a ?
    //   ^

    printf("[-]+<[");
        printf(">-<");
    printf("[-]]");

    printf(">[<+>-]");
}

u0 emit_negate_u8(){
    // a ?
    //   ^

    printf("<-[>+<-]>+[<->-]");
}

u0 emit_negate_u16(){
    // Add u16 value of 0 before value
    printf("[-]>[-]<");
    printf("<<[>>+<<-]>[>>+<<-]");
    printf("3>");
    emit_context.current_cell_index += 2;

    // Subtract
    emit_stdlib_function_call((u8*) "u16_sub", 2, 4);
}

u0 emit_negate_u32(){
    // Add u32 value of 0 before value

    printf("[-]>[-]>[-]>[-]7<");
    printf("[4>+4<-]>[4>+4<-]>[4>+4<-]>[4>+4<-]");
    printf("5>");

    emit_context.current_cell_index += 4;

    // Subtract
    emit_stdlib_function_call((u8*) "u32_sub", 4, 8);
}

u0 emit_bit_complement_u8(){
    // a ?
    //   ^

    printf("<[>+<-]>+[<->-]");
}

u0 emit_bit_complement_u16(){
    emit_stdlib_function_call((u8*) "u16_bit_neg", 2, 2);
}

u0 emit_bit_complement_u32(){
    emit_stdlib_function_call((u8*) "u32_bit_neg", 4, 4);
}

