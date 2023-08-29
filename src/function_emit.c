
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
#include "../include/emit.h"

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

static ErrorCode emit_if_like(Expression expression);
static ErrorCode emit_while(Expression expression);
static ErrorCode emit_do_while(Expression expression);

static u1 can_statement_break_current_level(u32 statement_index);
static u1 can_statements_break_current_level(u32 start_statement_i, u32 stop_statement_i);
static u1 can_loop_break(u32 statement_index);

static u0 emit_early_return_check(){
    if(emit_context.can_function_early_return){
        u32 offset = emit_context.current_cell_index - emit_context.incomplete_cell;

        // Ensure 'incomplete' cell is still true
        printf("%d<[%d>", offset, offset);

        // Remember to close at end of function
        emit_context.num_early_return_closes_needed++;
    }
}

static u0 emit_break_check(){
    if(emit_context.can_break){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_break_cell;

        // Ensure 'didnt_break_cell' cell is still true
        printf("%d<[%d>", offset, offset);

        // Remember to close at end of function
        emit_context.num_break_check_closes_needed++;
    }
}

static u0 emit_pre_loop_condition_early_return_check(){
    if(emit_context.can_function_early_return){
        // Allocate 1 cell for default condition value of false or eventual evaluated condition
        printf("[-]>");
        emit_context.current_cell_index++;

        // Copy 'incomplete' cell
        copy_cell_static(emit_context.incomplete_cell);

        // Go to copied cell
        printf("<");
        emit_context.current_cell_index--;

        // If function is still incomplete
        printf("[<");
        emit_context.current_cell_index--;
    }
}

static u0 emit_post_loop_condition_early_return_check(){
    if(emit_context.can_function_early_return){
        // Remain pointing at next available cell

        // Close if
        printf("[-]]");
    }
}

static u0 emit_pre_loop_condition_break_check(){
    if(emit_context.can_break){
        // Allocate 1 cell for default condition value of false or eventual evaluated condition
        printf("[-]>");
        emit_context.current_cell_index++;

        // Copy 'didnt_break_cell' cell
        copy_cell_static(emit_context.didnt_break_cell);

        // Go to copied cell
        printf("<");
        emit_context.current_cell_index--;

        // If loop hasn't been broken yet
        printf("[<");
        emit_context.current_cell_index--;
    }
}

static u0 emit_post_loop_condition_break_check(){
    if(emit_context.can_break){
        // Remain pointing at next available cell

        // Close if
        printf("[-]]");
    }
}

u0 emit_early_return_closes_until(u16 num_early_return_closes_to_leave){
    if(emit_context.num_early_return_closes_needed > num_early_return_closes_to_leave){
        u32 amount = emit_context.num_early_return_closes_needed - num_early_return_closes_to_leave;
        u32 offset = emit_context.current_cell_index - emit_context.incomplete_cell;

        // Zero temporary 'restoration_copy' cell
        printf("[-]");

        // Go to 'incomplete' cell
        printf("%d<", offset);

        // Move 'incomplete' cell to 'restoration_copy' cell
        printf("[%d>+%d<-]", offset, offset);

        // Close early return checks
        printf("%d]", amount);

        // Restore 'incomplete' cell from copy and go back to cell where came from
        printf("%d>[%d<+%d>-]", offset, offset, offset);

        // (emit_context.current_cell_index remains unchanged)

        emit_context.num_early_return_closes_needed = num_early_return_closes_to_leave;
    }
}

u0 emit_break_check_closes(){
    if(emit_context.num_break_check_closes_needed > 0){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_break_cell;

        // Zero temporary 'restoration_copy' cell
        printf("[-]");

        // Go to 'didnt_break_cell' cell
        printf("%d<", offset);

        // Move 'didnt_break_cell' cell to 'restoration_copy' cell
        printf("[%d>+%d<-]", offset, offset);

        // Close early return checks
        printf("%d]", emit_context.num_break_check_closes_needed);

        // Restore 'didnt_break_cell' cell from copy and go back to cell where came from
        printf("%d>[%d<+%d>-]", offset, offset, offset);

        // (emit_context.current_cell_index remains unchanged)

        emit_context.num_break_check_closes_needed = 0;
    }
}

static ErrorCode emit_body(u32 start_statement_i, u32 stop_statement_i){
    for(u32 i = start_statement_i; i < stop_statement_i; i++){
        Expression expression = expressions[statements[i]];
        emit_context.current_statement = i;

        // Skip over contained statements
        switch(expression.kind){
        case EXPRESSION_IF:
            if(emit_if_like(expression)) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            if(emit_if_like(expression)) return 1;
            i += operands[expression.ops + 1] + operands[expression.ops + 2];
            break;
        case EXPRESSION_WHILE:
            if(emit_while(expression)) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_DO_WHILE:
            if(emit_do_while(expression)) return 1;
            i += operands[expression.ops + 1];
            break;
        default: {
                u32 result_type = expression_emit(expression);
                if(result_type >= TYPES_CAPACITY) return 1;

                if(result_type != u0_type){
                    printf("\nerror on line %d: Statement result ignored\n", u24_unpack(expression.line));
                    return 1;
                }
                
                if(expression.kind == EXPRESSION_RETURN || expression.kind == EXPRESSION_BREAK){
                    return 0;
                }
            }
        }
    }

    return 0;
}

static ErrorCode emit_if_like(Expression expression){
    // Emits code for if/if-else statements

    u32 starting_cell_index = emit_context.current_cell_index;
    
    // Evaluate condition
    u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
    if(condition_type >= TYPES_CAPACITY) return 1;

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'if' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    u1 has_else = expression.kind == EXPRESSION_IF_ELSE;

    if(has_else){
        // Allocate 'should_run_else' cell
        printf("[-]+");
    }

    // Go to condition cell
    printf("<");
    emit_context.current_cell_index--;

    // If condition
    printf("[");

    // Emit 'if' body
    emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1);

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    // Go to 'condition' cell
    if(has_else){
        // Zero 'should_run_else' cell on the way
        printf(">[-]<");
    }

    // End if
    printf("[-]]");

    // Handle else
    if(has_else){
        // Change starting cell index to 'should_run_else' cell
        starting_cell_index++;

        // Go to 'should_run_else' cell
        printf(">");
        emit_context.current_cell_index++;

        // If 'should_run_else' cell
        printf("[");

        // Emit 'else' body
        emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 2] + 1);

        // Deallocate variables
        if(emit_context.current_cell_index > starting_cell_index){
            printf("%d<", emit_context.current_cell_index - starting_cell_index);
            emit_context.current_cell_index = starting_cell_index;
        }

        // End if
        printf("[-]]");

        // Go to next available cell
        printf("<");
        emit_context.current_cell_index--;
    }

    emit_break_check();
    emit_early_return_check();
    return 0;
}

static ErrorCode emit_while(Expression expression){
    // Emits code for if/if-else statements

    u1 was_breakable = emit_context.can_break;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_num_break_check_closes_needed = emit_context.num_break_check_closes_needed;

    emit_context.can_break = can_loop_break(emit_context.current_statement);

    if(emit_context.can_break){
        // Allocate 'didnt_break_cell'
        printf("[-]+>");
        emit_context.didnt_break_cell = emit_context.current_cell_index++;
    }

    u32 starting_cell_index = emit_context.current_cell_index;
    
    // Evaluate condition
    emit_pre_loop_condition_early_return_check();
    u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
    if(condition_type >= TYPES_CAPACITY) return 1;
    emit_post_loop_condition_early_return_check();

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'while' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // While 'condition' cell
    printf("[");

    // Emit 'while' body
    u16 early_return_watermark = emit_context.num_early_return_closes_needed;
    emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1);
    emit_break_check_closes();
    emit_early_return_closes_until(early_return_watermark);

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    // Re-evaluate condition (should never fail)
    emit_pre_loop_condition_early_return_check();
    emit_pre_loop_condition_break_check();
    expression_emit(expressions[operands[expression.ops]]);
    emit_post_loop_condition_break_check();
    emit_post_loop_condition_early_return_check();

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // End while
    printf("]");

    // Deallocate 'didnt_break_cell' if used
    if(emit_context.can_break){
        printf("<");
        emit_context.current_cell_index--;
    }

    emit_context.can_break = was_breakable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.num_break_check_closes_needed = old_num_break_check_closes_needed;

    emit_break_check();
    emit_early_return_check();
    return 0;
}

static ErrorCode emit_do_while(Expression expression){
    // Emits code for if/if-else statements

    u1 was_breakable = emit_context.can_break;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_num_break_check_closes_needed = emit_context.num_break_check_closes_needed;

    emit_context.can_break = can_loop_break(emit_context.current_statement);

    if(emit_context.can_break){
        // Allocate 'didnt_break_cell'
        printf("[-]+>");
        emit_context.didnt_break_cell = emit_context.current_cell_index++;
    }

    u32 starting_cell_index = emit_context.current_cell_index;

    // Set 'condition' cell to true
    printf("[-]+");

    // While 'condition' cell
    printf("[");
    
    // Emit 'do-while' body
    u16 early_return_watermark = emit_context.num_early_return_closes_needed;
    emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1);
    emit_break_check_closes();
    emit_early_return_closes_until(early_return_watermark);

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    // Evaluate condition
    emit_pre_loop_condition_early_return_check();
    emit_pre_loop_condition_break_check();
    u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
    if(condition_type >= TYPES_CAPACITY) return 1;
    emit_post_loop_condition_break_check();
    emit_post_loop_condition_early_return_check();

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'do-while' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // End while
    printf("]");
    
    // Deallocate 'didnt_break_cell' if used
    if(emit_context.can_break){
        printf("<");
        emit_context.current_cell_index--;
    }

    emit_context.can_break = was_breakable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.num_break_check_closes_needed = old_num_break_check_closes_needed;

    emit_break_check();
    emit_early_return_check();
    return 0;
}

u1 has_return_in_region(u32 start_statement_i, u32 stop_statement_i){
    for(u32 i = start_statement_i; i < stop_statement_i; i++){
        if(expressions[statements[i]].kind == EXPRESSION_RETURN){
            return true;
        }
    }

    return false;
}

u1 can_function_early_return(u32 function_index){
    Function function = functions[function_index];

    for(u32 i = function.begin; i < function.begin + function.num_stmts; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_IF:
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
            if(has_return_in_region(i + 1, i + operands[expression.ops + 1] + 1)) return true;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            if(has_return_in_region(i + 1, i + operands[expression.ops + 1] + 1)) return true;
            i += operands[expression.ops + 1];
            if(has_return_in_region(i + 1, i + operands[expression.ops + 2] + 1)) return true;
            i += operands[expression.ops + 2];
            break;
        case EXPRESSION_RETURN:
            return false;
        }
    }

    return false;
}

static u1 can_statements_break_current_level(u32 start_statement_i, u32 stop_statement_i){
    for(u32 i = start_statement_i; i < stop_statement_i; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
            i += operands[expression.ops + 1];
            break;
        default:
            if(can_statement_break_current_level(i)){
                return true;
            }
        }
    }

    return false;
}

static u1 can_statement_break_current_level(u32 statement_index){
    Expression expression = expressions[statements[statement_index]];

    switch(expression.kind){
    case EXPRESSION_IF: {
            u32 num_then = operands[expression.ops + 1];
            return can_statements_break_current_level(statement_index + 1, statement_index + 1 + num_then);
        }
        break;
    case EXPRESSION_IF_ELSE: {
            u32 num_then = operands[expression.ops + 1];
            u32 num_else = operands[expression.ops + 2];
            return can_statements_break_current_level(statement_index + 1, statement_index + 1 + num_then + num_else);
        }
        break;
    case EXPRESSION_BREAK:
        return true;
    default:
        return false;
    }
}

static u1 can_loop_break(u32 statement_index){
    Expression container = expressions[statements[statement_index]];

    switch(container.kind){
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE: {
            u32 num_inside = operands[container.ops + 1];
            return can_statements_break_current_level(statement_index + 1, statement_index + 1 + num_inside);
        }
        break;
    default:
        return can_statement_break_current_level(statement_index);
    }
}

ErrorCode function_emit(u32 function_index, u32 start_function_cell_index, u32 start_current_cell_index){
    Function function = functions[function_index];
    u1 can_early_return = can_function_early_return(function_index);

    EmitContext old_emit_context = emit_context;

    emit_context = (EmitContext){
        .function = function_index,
        .function_cell_index = start_function_cell_index,
        .current_cell_index = start_current_cell_index,
        .current_statement = function.begin,
        .in_recursive_function = function.is_recursive,
        .can_function_early_return = can_early_return,
        .num_early_return_closes_needed = 0,
        .incomplete_cell = -1,
        .can_break = 0,
        .didnt_break_cell = 0,
    };
    
    // Allocate 'incomplete' cell if function can return early
    if(can_early_return){
        printf("[-]+>");
        emit_context.incomplete_cell = emit_context.current_cell_index++;
    }

    if(emit_body(function.begin + function.arity, function.begin + function.num_stmts)){
        return 1;
    }

    emit_early_return_closes_until(0);

    if(emit_context.current_cell_index > start_function_cell_index){
        printf("%d<", emit_context.current_cell_index - start_function_cell_index);
    }

    emit_context = old_emit_context;
    emit_context.current_cell_index = start_function_cell_index;
    return 0;
}

