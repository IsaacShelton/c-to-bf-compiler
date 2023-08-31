
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
#include "../include/expression_print.h"

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
static ErrorCode emit_for(Expression expression);

static u1 can_statement_break_current_level(u32 statement_index);
static u1 can_statements_break_current_level(u32 start_statement_i, u32 stop_statement_i);
static u1 can_loop_break(u32 statement_index);
static u1 can_statement_continue_current_level(u32 statement_index);
static u1 can_statements_continue_current_level(u32 start_statement_i, u32 stop_statement_i);
static u1 can_loop_continue(u32 statement_index);

static ErrorCode add_close_needed(CloseNeeded close_needed){
    if(num_closes_needed >= CLOSES_NEEDED_CAPCAITY){
        printf("\nout of memory: Exceeded maximum number of pending check closes\n");
        return 1;
    }

    closes_needed[num_closes_needed++] = close_needed;
    return 0;
}

static ErrorCode emit_early_return_check(){
    if(emit_context.can_function_early_return){
        u32 offset = emit_context.current_cell_index - emit_context.incomplete_cell;

        // Allocate temporary cell, and ensure 'incomplete' cell is still true
        printf("[-]%d<[%d>", offset, offset);

        return add_close_needed(CLOSE_NEEDED_FOR_EARLY_RETURN_CHECK);
    } else {
        return 0;
    }
}

static ErrorCode emit_break_check(){
    if(emit_context.can_break){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_break_cell;

        // Allocate temporary cell, and ensure 'didnt_break_cell' cell is still true
        printf("[-]%d<[%d>", offset, offset);

        return add_close_needed(CLOSE_NEEDED_FOR_BREAK_CHECK);
    } else {
        return 0;
    }
}

static ErrorCode emit_continue_check(){
    if(emit_context.can_continue){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;

        // Allocate temporary cell, and ensure 'didnt_continue_cell' cell is still true
        printf("[-]%d<[%d>", offset, offset);

        return add_close_needed(CLOSE_NEEDED_FOR_CONTINUE_CHECK);
    } else {
        return 0;
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

ErrorCode emit_close_checks_until(u32 waterline){
    while(num_closes_needed > waterline){
        CloseNeeded close_needed = closes_needed[--num_closes_needed];

        u32 offset;

        switch(close_needed){
        case CLOSE_NEEDED_FOR_EARLY_RETURN_CHECK:
            offset = emit_context.current_cell_index - emit_context.incomplete_cell;
            break;
        case CLOSE_NEEDED_FOR_BREAK_CHECK:
            offset = emit_context.current_cell_index - emit_context.didnt_break_cell;
            break;
        case CLOSE_NEEDED_FOR_CONTINUE_CHECK:
            offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;
            break;
        default:
            printf("\nerror: emit_close_checks_until() got unknown close kind\n");
            return 1;
        }

        u32 amount = 1;

        while(num_closes_needed > waterline && closes_needed[num_closes_needed - 1] == close_needed){
            num_closes_needed--;
            amount++;
        }

        // Zero temporary 'restoration_copy' cell
        printf("[-]");

        // Go to target cell
        printf("%d<", offset);

        // Move target cell to 'restoration_copy' cell
        printf("[%d>+%d<-]", offset, offset);

        // Close checks of same kind
        printf("%d]", amount);

        // Restore target cell from copy and go back to cell where came from
        printf("%d>[%d<+%d>-]", offset, offset, offset);

        // (emit_context.current_cell_index remains unchanged)
    }

    return 0;
}

static ErrorCode emit_body(u32 start_statement_i, u32 stop_statement_i){
    u32 closes_needed_waterline = num_closes_needed;

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
        case EXPRESSION_FOR:
            if(emit_for(expression)) return 1;
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
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
                
                if(expression.kind == EXPRESSION_RETURN || expression.kind == EXPRESSION_BREAK || expression.kind == EXPRESSION_CONTINUE){
                    i = stop_statement_i;
                }
            }
        }
    }

    emit_close_checks_until(closes_needed_waterline);
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
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1)){
        return 1;
    }

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
        if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 2] + 1)){
            return 1;
        }

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

    return emit_break_check() || emit_continue_check() || emit_early_return_check();
}

static u0 emit_reset_didnt_continue(){
    if(emit_context.can_continue){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;

        // Set 'didnt_continue_cell' to true
        printf("%d<[-]+%d>", offset, offset);
    }
}

static u0 enter_maybe_breakable_continuable_region(Expression expression, u32 inner_variable_offset_ops_offset){
    emit_context.can_break = can_loop_break(emit_context.current_statement);
    emit_context.can_continue = can_loop_continue(emit_context.current_statement);

    // Start inner variable offset at zero
    operands[expression.ops + 2] = 0;

    if(emit_context.can_break){
        // Allocate 'didnt_break_cell'
        printf("[-]+>");
        emit_context.didnt_break_cell = emit_context.current_cell_index++;

        // Increase inner variable offset
        operands[expression.ops + inner_variable_offset_ops_offset]++;
    }

    if(emit_context.can_continue){
        // Allocate 'didnt_continue_cell'
        printf("[-]+>");
        emit_context.didnt_continue_cell = emit_context.current_cell_index++;

        // Increase inner variable offset
        operands[expression.ops + inner_variable_offset_ops_offset]++;
    }
}

static u0 exit_maybe_breakable_continuable_region(){
    // Deallocate 'didnt_continue_cell' if used
    if(emit_context.can_continue){
        printf("<");
        emit_context.current_cell_index--;
    }

    // Deallocate 'didnt_break_cell' if used
    if(emit_context.can_break){
        printf("<");
        emit_context.current_cell_index--;
    }
}

static ErrorCode emit_while(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_didnt_continue_cell = emit_context.didnt_continue_cell;

    enter_maybe_breakable_continuable_region(expression, 2);

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
    emit_reset_didnt_continue();

    // Emit 'while' body
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1)){
        return 1;
    }

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

    exit_maybe_breakable_continuable_region();

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return emit_break_check() || emit_continue_check() || emit_early_return_check();
}

static ErrorCode emit_do_while(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_didnt_continue_cell = emit_context.didnt_continue_cell;

    enter_maybe_breakable_continuable_region(expression, 2);

    u32 starting_cell_index = emit_context.current_cell_index;

    // Set 'condition' cell to true
    printf("[-]+");

    // While 'condition' cell
    printf("[");
    emit_reset_didnt_continue();
    
    // Emit 'do-while' body
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1)){
        return 1;
    }

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
    
    exit_maybe_breakable_continuable_region();

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return emit_break_check() || emit_continue_check() || emit_early_return_check();
}

static ErrorCode emit_for(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_didnt_continue_cell = emit_context.didnt_continue_cell;

    u32 num_pre = operands[expression.ops];
    u32 num_post = operands[expression.ops + 2];
    u32 num_inside = operands[expression.ops + 3];

    u32 pre_start_statement = emit_context.current_statement + 1;
    u32 post_start_statement = pre_start_statement + num_pre;
    u32 inside_start_statement = post_start_statement + num_post;

    u32 pre_starting_cell_index = emit_context.current_cell_index;

    // Emit pre-statements
    if(emit_body(pre_start_statement, pre_start_statement + num_pre)){
        return 1;
    }

    enter_maybe_breakable_continuable_region(expression, 4);

    u32 starting_cell_index = emit_context.current_cell_index;
    
    // Evaluate condition
    emit_pre_loop_condition_early_return_check();
    u32 condition_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(condition_type >= TYPES_CAPACITY) return 1;
    emit_post_loop_condition_early_return_check();

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'for' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // While 'condition' cell
    printf("[");
    emit_reset_didnt_continue();

    // Emit 'for' body
    if(emit_body(inside_start_statement, inside_start_statement + num_inside)){
        return 1;
    }

    // Evaluate post-statements
    if(emit_body(post_start_statement, post_start_statement + num_post)){
        return 1;
    }

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    // Re-evaluate condition (should never fail)
    emit_pre_loop_condition_early_return_check();
    emit_pre_loop_condition_break_check();
    expression_emit(expressions[operands[expression.ops + 1]]);
    emit_post_loop_condition_break_check();
    emit_post_loop_condition_early_return_check();

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // End while
    printf("]");

    exit_maybe_breakable_continuable_region();

    // Deallocate pre-statement variables
    if(emit_context.current_cell_index > pre_starting_cell_index){
        printf("%d<", emit_context.current_cell_index - pre_starting_cell_index);
        emit_context.current_cell_index = pre_starting_cell_index;
    }

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return emit_break_check() || emit_continue_check() || emit_early_return_check();
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
        case EXPRESSION_IF_ELSE: {
                u32 num_then = operands[expression.ops + 1];
                u32 num_else = operands[expression.ops + 2];
                if(has_return_in_region(i + 1, i + num_then + num_else + 1)) return true;
                i += num_then + num_else;
            }
            break;
        case EXPRESSION_FOR: {
                u32 num_pre = operands[expression.ops];
                u32 num_post = operands[expression.ops + 2];
                u32 len = operands[expression.ops + 3];

                if(has_return_in_region(i + 1, i + num_pre + num_post + len + 1)) return true;
                i += num_pre + num_post + len;
            }
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
        case EXPRESSION_FOR:
            if(can_statement_break_current_level(i)){
                return true;
            }
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
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
    case EXPRESSION_FOR: {
            u32 num_pre = operands[expression.ops];
            return can_statements_break_current_level(statement_index + 1, statement_index + num_pre + 1);
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

static u1 can_statements_continue_current_level(u32 start_statement_i, u32 stop_statement_i){
    for(u32 i = start_statement_i; i < stop_statement_i; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_FOR:
            if(can_statement_continue_current_level(i)){
                return true;
            }
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        default:
            if(can_statement_continue_current_level(i)){
                return true;
            }
        }
    }

    return false;
}

static u1 can_statement_continue_current_level(u32 statement_index){
    Expression expression = expressions[statements[statement_index]];

    switch(expression.kind){
    case EXPRESSION_IF: {
            u32 num_then = operands[expression.ops + 1];
            return can_statements_continue_current_level(statement_index + 1, statement_index + 1 + num_then);
        }
        break;
    case EXPRESSION_IF_ELSE: {
            u32 num_then = operands[expression.ops + 1];
            u32 num_else = operands[expression.ops + 2];
            return can_statements_continue_current_level(statement_index + 1, statement_index + 1 + num_then + num_else);
        }
        break;
    case EXPRESSION_FOR: {
            u32 num_pre = operands[expression.ops];
            return can_statements_continue_current_level(statement_index + 1, statement_index + num_pre + 1);
        }
        break;
    case EXPRESSION_CONTINUE:
        return true;
    default:
        return false;
    }
}

static u1 can_loop_continue(u32 statement_index){
    Expression container = expressions[statements[statement_index]];

    switch(container.kind){
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE: {
            u32 num_inside = operands[container.ops + 1];
            return can_statements_continue_current_level(statement_index + 1, statement_index + 1 + num_inside);
        }
        break;
    default:
        return can_statement_continue_current_level(statement_index);
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
        .incomplete_cell = -1,
        .can_break = false,
        .didnt_break_cell = -1,
        .can_continue = false,
        .didnt_continue_cell = -1,
    };
    
    // Allocate 'incomplete' cell if function can return early
    if(can_early_return){
        printf("[-]+>");
        emit_context.incomplete_cell = emit_context.current_cell_index++;
    }

    if(emit_body(function.begin + function.arity, function.begin + function.num_stmts)){
        return 1;
    }

    if(emit_context.current_cell_index > start_function_cell_index){
        printf("%d<", emit_context.current_cell_index - start_function_cell_index);
    }

    emit_context = old_emit_context;
    emit_context.current_cell_index = start_function_cell_index;
    return 0;
}

