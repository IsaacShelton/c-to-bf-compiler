
#include <stdio.h>
#include "../include/function_emit.h"
#include "../include/function.h"
#include "../include/storage.h"
#include "../include/expression_emit.h"
#include "../include/builtin_types.h"
#include "../include/type_print.h"
#include "../include/type_emit.h"
#include "../include/variable_find.h"
#include "../include/emit_context.h"

// ============= Inlined function calls =============
// Have layout like: `global1 global2 unknown1 unknown2 return_value1 return_value2 return_value3 param1 param2 param3 param4 local1 local2`
//                                                                                                                            ^
// on the tape.
// They are told their current position on the tape.
//
// Calling: ----------------------------
// Return value space and parameters are pushed on to the tape.
// The cell pointer is then left pointing at the next available cell.
// Returning: --------------------------
// Parameters and local variables will be popped from the tape.
// The return value will remain intact as a value on the tape.
// The cell pointer is decremented to be the next available cell.
//
// For example, the `put` function is implemented as:
// <.
// To call the put function, we push the return value (no cells) and paremeters (single cell) on to the tape,
// with the cell pointer pointing to the next available cell.
// 65+>
// The function contents are inlined (this may require knowning the cell pointer at compile time in some cases),
// <.
// The function returns, by getting rid of the parameters (1 cell in this case) and pointing to the next available cell, while retaining the
// return value (0 cells in this case)
// No additional code is necessary for this example

// 10 ? ? ?
//    ^
// 10 ? ? ?
//  ^ -------------- and outputs 10
// ? ? ? ?
// ^ -------------- after return

// The function is responsible for returning, the caller is responsible for calling.

// ============= Recursive functions calls =============
// Have stack layout like: `unknown1 unknown2 return_value1 return_value2 return_value3 param1 param2 param3 param4 local1 local2`
// Have tape layout like: `global1 global2`
// Caller pushes uninitialized space for return values (on to the stack)
// Caller pushes return block address (on to the stack)
// Caller pushes parameters with initialized values (on to the stack)
// Caller pushes target block address (on to the stack)
// Control flow is given to function dispatcher
// Tape pointer should be pointing to the beginning of the stack pointer

// The callee then returns by popping it's local variables and parameters,
// Leaving the return block address at the stop of the stack
// Tape pointer should be pointing to the beginning of the stack pointer
// The caller can now use the return value by copying it to the tape

// Whether a function is recursive or not will be determined by the following:
// Create an array with the amount of recursive calls for each function (all calls are assumed to be recursive to start)
// Create array of arrows, each arrow is from_function_index->to_function_index
/*
u16 num_outgoing[MAX_FUNCTIONS];
typedef struct { u16 source; u16 destination; } Arrow;
Arrow arrows[MAX_ARROWS];
u1 changed = 1;
while(changed){
    changed = 0;

    for(int i = 0; i < arrows.length; i++){
        u16 destination = arrows[i].destination;

        if(num_outgoing[destination] == 0){
            for(int j = 0; j < arrows.length; j++){
                if(arrows[j].destination == destination){
                    num_outgoing[arrow.source]--;
                    memove(arrows[j], &arrows[j + 1], sizeof arrows - j);
                    j--;
                }
            }
            changed = 1;
        }
    }
}
for(int i = 0; i < functions.length; i++){
    functions[i].is_recursive = (num_outgoing[i] == 0);
}
*/

/*
GLOBAL_VARIABLES globals;
u32 stack_pointer;
u8[MAX_U32] stack;

// Memory never used (stack_pointer of zero means exit)
stack[0] = 0;
stack[1] = 0;

// Entry point block
stack[2] = 0;
stack[3] = 1;

// Size of starting block address
stack_pointer = 2;

while(stack_pointer != 0){
    const u16 block_address = u16(stack[stack_pointer], stack[stack_pointer + 1]);

    switch(block_address){
    case 1: // Main Function Block 1
        // Write return address
        stack[stack_pointer] = 0;
        stack[stack_pointer + 1] = 3;
        // Write parameters (none)
        // Write callee block address
        stack[stack_pointer] = 0;
        stack[stack_pointer + 1] = 2;
        // Increase stack pointer by the size of what we appended
        stack_pointer += 4; // Call Function 2 Block 1
        break;
    case 2: // Function 2 Block 1
        printf("Hello World\n");
        // Do nothing, return
        break;
    case 3: // Main Function Block 2
        printf("Bye world!\n");
        // Do nothing, return
        break;
    }

    stack_pointer -= 2;
}
*/

static u32 function_sizeof_parameters_or_max(Function function){
    u32 size = 0;

    for(u32 i = 0; i < function.arity; i++){
        Expression declare = expressions[statements[function.begin + i]];
        u32 type = operands[declare.ops];
        u32 type_size = type_sizeof_or_max(type);

        if(type_size == -1){
            return -1;
        }

        size += type_size;
    }

    return size;
}

u32 function_emit(Function function, u32 start_function_cell_index, u32 start_current_cell_index){
    EmitContext old_emit_context = emit_context;

    emit_context = (EmitContext){
        .function_cell_index = start_function_cell_index,
        .current_cell_index = start_current_cell_index,
        .function_begin_statement = function.begin,
        .current_statement = function.begin,
        .in_recursive_function = function.is_recursive,
    };

    for(u32 i = function.arity; i < function.num_stmts; i++){
        Expression expression = expressions[statements[function.begin + i]];
        emit_context.current_statement = function.begin + i;
        u32 result_type = expression_emit(expression);

        if(result_type >= TYPES_CAPACITY){
            return 1;
        }

        if(result_type != u0_type){
            printf("\nError: Statement result ignored\n");
            return 1;
        }
    }

    /*
    printf("\nCurrent cell index: %d\n", emit_context.current_cell_index);
    printf("\nInside function: ");
    print_aux_cstr(function.name);
    printf("\n");
    u32 name = aux_cstr_alloc((u8*) "variable");
    if(name >= AUX_CAPACITY){
        printf("\nerror: allocating string\n");
        return 1;
    }
    Variable variable = variable_find(name);

    printf("\nGot: %s ", variable.defined ? "defined" : "undefined");
    if(variable.defined){
        print_aux_cstr(variable.name);
        printf(" ");
        type_print(types[variable.type]);
        printf(" ");
        printf("depth:%d ", variable.depth);
        printf("declaration:%d ", variable.declaration);
        printf("location.on_stack:%s ", variable.location.on_stack ? "true" : "false");
        printf("location.location:%d ", variable.location.location);
    }
    printf("\n");
    */


    if(emit_context.current_cell_index > start_function_cell_index){
        printf("%d<", emit_context.current_cell_index - start_function_cell_index);
    }

    emit_context = old_emit_context;
    emit_context.current_cell_index = start_function_cell_index;
    return 0;
}

