
#include <stdio.h>
#include <assert.h>
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
#include "../include/stack_driver.h"

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
static ErrorCode emit_switch(Expression expression);
static ErrorCode emit_case(Expression expression);

static ErrorCode emit_if_like_stack(Expression expression);
static ErrorCode emit_if_like_tape(Expression expression);
static ErrorCode emit_while_stack(Expression expression);
static ErrorCode emit_while_tape(Expression expression);
static ErrorCode emit_do_while_stack(Expression expression);
static ErrorCode emit_do_while_tape(Expression expression);
static ErrorCode emit_for_stack(Expression expression);
static ErrorCode emit_for_tape(Expression expression);
static ErrorCode emit_conditionless_block(Expression expression);
static ErrorCode emit_switch_stack(Expression expression);
static ErrorCode emit_switch_tape(Expression expression);
static ErrorCode emit_case_stack(Expression expression);
static ErrorCode emit_case_tape(Expression expression);

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

static u0 emit_pre_case_fallthrough_check(){
    // Allocate default result (true)
    printf("[-]+>");
    emit_context.current_cell_index++;

    // Copy 'fell_through_cell'
    copy_cell_static(emit_context.fell_through_cell);

    // Negate 'fell_through_cell'
    emit_not_u1();

    // Go to '!fell_through_cell'
    printf("<");
    emit_context.current_cell_index--;

    // If hasn't fallen through, evalute case value instead of
    // default value
    printf("[<");
    emit_context.current_cell_index--;
}

static u0 emit_post_case_fallthrough_check(){
    // Remain pointing at next available cell
    // End if
    printf("[-]]");
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

static ErrorCode emit_body(u32 start_statement_i, u32 stop_statement_i, u1 allow_cases){
    u32 closes_needed_waterline = num_closes_needed;

    for(u32 i = start_statement_i; i < stop_statement_i; i++){
        Expression expression = expressions[statements[i]];
        emit_context.current_statement = i;

        // Skip over contained statements
        switch(expression.kind){
        case EXPRESSION_IF:
            if(emit_if_like(expression) != 0) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            if(emit_if_like(expression) != 0) return 1;
            i += operands[expression.ops + 1] + operands[expression.ops + 2];
            break;
        case EXPRESSION_WHILE:
            if(emit_while(expression) != 0) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_FOR:
            if(emit_for(expression) != 0) return 1;
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        case EXPRESSION_DO_WHILE:
            if(emit_do_while(expression) != 0) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_CONDITIONLESS_BLOCK:
            if(emit_conditionless_block(expression) != 0) return 1;
            i += expression.ops;
            break;
        case EXPRESSION_SWITCH:
            if(emit_switch(expression) != 0) return 1;
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_CASE:
            if(!allow_cases){
                expression_print_cannot_use_case_here_error(expression);
                return 1;
            }
            if(emit_case(expression) != 0) return 1;
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

    return emit_close_checks_until(closes_needed_waterline);
}

static ErrorCode emit_body_scoped(u32 start_statement_i, u32 stop_statement_i, u1 allow_cases){
    u32 starting_cell_index = emit_context.current_cell_index;

    // Emit body
    if(emit_body(start_statement_i, stop_statement_i, allow_cases) != 0){
        return 1;
    }

    // Deallocate locally scoped variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    return 0;
}


static ErrorCode emit_if_like(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_if_like_stack(expression);
    } else {
        return emit_if_like_tape(expression);
    }
}

static ErrorCode emit_if_like_stack(Expression expression){
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

    u32 then_block = emit_settings.next_basicblock_id++;
    u32 else_block = emit_settings.next_basicblock_id++;
    u32 continuation_block = has_else ? emit_settings.next_basicblock_id++ : else_block;

    u32 pushed = emit_end_basicblock_jump_conditional(then_block, else_block);

    // Then block
    emit_start_basicblock_landing(then_block, pushed);

    // Emit 'if' body
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
        return 1;
    }

    if(emit_settings.in_basicblock){
        emit_end_basicblock_jump_compatible(continuation_block, pushed);
    }

    if(has_else){
        // Else block
        emit_start_basicblock_landing(else_block, pushed);

        // Emit 'else' body
        if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 2] + 1, false) != 0){
            return 1;
        }

        if(emit_settings.in_basicblock){
            emit_end_basicblock_jump_compatible(continuation_block, pushed);
        }
    }

    // Continuation block
    emit_start_basicblock_landing(continuation_block, pushed);
    return 0;
}

static ErrorCode emit_if_like_tape(Expression expression){
    // Emits code for if/if-else statements
    
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
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
        return 1;
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
        // Go to 'should_run_else' cell
        printf(">");
        emit_context.current_cell_index++;

        // If 'should_run_else' cell
        printf("[");

        // Emit 'else' body
        if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 2] + 1, false) != 0){
            return 1;
        }

        // End if
        printf("[-]]");

        // Go to next available cell
        printf("<");
        emit_context.current_cell_index--;
    }

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
}

static u0 emit_reset_didnt_continue(){
    if(emit_context.can_continue){
        u32 offset = emit_context.current_cell_index - emit_context.didnt_continue_cell;

        // Set 'didnt_continue_cell' to true
        printf("%d<[-]+%d>", offset, offset);
    }
}

static u0 enter_maybe_breakable_continuable_loop_tape(Expression expression, u32 inner_variable_offset_ops_offset){
    emit_context.can_break = can_loop_break(emit_context.current_statement);
    emit_context.can_continue = can_loop_continue(emit_context.current_statement);

    operands[expression.ops + inner_variable_offset_ops_offset] = 0;

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

static u0 exit_maybe_breakable_continuable_loop(){
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

static u0 enter_switch(){
    emit_context.can_break = true;

    // Allocate 'didnt_break_cell'
    printf("[-]+>");
    emit_context.didnt_break_cell = emit_context.current_cell_index++;

    // Allocate 'fell_through_cell'
    printf("[-]>");
    emit_context.fell_through_cell = emit_context.current_cell_index++;
}

static u0 exit_switch(){
    // Deallocate 'didnt_break_cell'
    printf("<");
    emit_context.current_cell_index--;
}

static ErrorCode emit_while(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_while_stack(expression);
    } else {
        return emit_while_tape(expression);
    }
}

static ErrorCode emit_while_stack(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    JumpContext old_break_context = emit_settings.break_basicblock_context;
    JumpContext old_continue_context = emit_settings.continue_basicblock_context;

    // Evaluate condition
    u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
    if(condition_type >= TYPES_CAPACITY) return 1;

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'while' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    u32 then_block = emit_settings.next_basicblock_id++;
    u32 continuation_block = emit_settings.next_basicblock_id++;
    u32 pushed = emit_end_basicblock_jump_conditional(then_block, continuation_block);

    emit_context.can_break = can_loop_break(emit_context.current_statement);
    emit_context.can_continue = can_loop_continue(emit_context.current_statement);

    if(emit_context.can_break){
        emit_settings.break_basicblock_context = (JumpContext){
            .basicblock_id = continuation_block,
            .num_cells_input = pushed,
        };
    }

    if(emit_context.can_continue){
        // Create special separate block if using `continue`
        u32 condition_block = emit_settings.next_basicblock_id++;

        // Condition block
        emit_start_basicblock_landing(condition_block, pushed);

        // Re-evaluate condition (this should never fail)
        (u0) expression_emit(expressions[operands[expression.ops]]);

        // Either redo or continue onwards
        if(emit_settings.in_basicblock){
            (u0) emit_end_basicblock_jump_conditional(then_block, continuation_block);
        }

        emit_settings.continue_basicblock_context = (JumpContext){
            .basicblock_id = condition_block,
            .num_cells_input = pushed,
        };
    }

    // Then block
    emit_start_basicblock_landing(then_block, pushed);

    // Emit 'while' body
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
        return 1;
    }

    // Either redo or continue onwards
    if(emit_settings.in_basicblock){
        // Re-evaluate condition (this should never fail)
        (u0) expression_emit(expressions[operands[expression.ops]]);

        (u0) emit_end_basicblock_jump_conditional(then_block, continuation_block);
    }

    // Continuation block
    emit_start_basicblock_landing(continuation_block, pushed);
    emit_settings.break_basicblock_context = old_break_context;
    emit_settings.continue_basicblock_context = old_continue_context;
    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    return 0;
}

static ErrorCode emit_while_tape(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_didnt_continue_cell = emit_context.didnt_continue_cell;

    enter_maybe_breakable_continuable_loop_tape(expression, 2);

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
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
        return 1;
    }

    // Re-evaluate condition (should never fail)
    emit_pre_loop_condition_early_return_check();
    emit_pre_loop_condition_break_check();
    (u0) expression_emit(expressions[operands[expression.ops]]);
    emit_post_loop_condition_break_check();
    emit_post_loop_condition_early_return_check();

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // End while
    printf("]");

    exit_maybe_breakable_continuable_loop();

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
}

static ErrorCode emit_do_while(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_do_while_stack(expression);
    } else {
        return emit_do_while_tape(expression);
    }
}

static ErrorCode emit_do_while_stack(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    JumpContext old_break_context = emit_settings.break_basicblock_context;
    JumpContext old_continue_context = emit_settings.continue_basicblock_context;

    u32 then_block = emit_settings.next_basicblock_id++;
    u32 continuation_block = emit_settings.next_basicblock_id++;
    u32 pushed = emit_end_basicblock_jump(then_block);

    emit_context.can_break = can_loop_break(emit_context.current_statement);
    emit_context.can_continue = can_loop_continue(emit_context.current_statement);

    if(emit_context.can_break){
        emit_settings.break_basicblock_context = (JumpContext){
            .basicblock_id = continuation_block,
            .num_cells_input = pushed,
        };
    }

    if(emit_context.can_continue){
        // Create special separate block if using `continue`
        u32 condition_block = emit_settings.next_basicblock_id++;

        // Condition block
        emit_start_basicblock_landing(condition_block, pushed);

        // Evaluate condition
        u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
        if(condition_type >= TYPES_CAPACITY) return 1;

        if(condition_type != u1_type){
            printf("\nerror on line %d: Expected 'do-while' condition to be 'u1', got '", u24_unpack(expression.line));
            type_print(types[condition_type]);
            printf("'\n");
            return 1;
        }

        // Either redo or continue onwards
        if(emit_settings.in_basicblock){
            (u0) emit_end_basicblock_jump_conditional(then_block, continuation_block);
        }

        emit_settings.continue_basicblock_context = (JumpContext){
            .basicblock_id = condition_block,
            .num_cells_input = pushed,
        };
    }

    // Then block
    emit_start_basicblock_landing(then_block, pushed);

    // Emit 'do-while' body
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
        return 1;
    }

    // Either redo or continue onwards
    if(emit_settings.in_basicblock){
        // Evaluate condition
        u32 condition_type = expression_emit(expressions[operands[expression.ops]]);
        if(condition_type >= TYPES_CAPACITY) return 1;

        if(condition_type != u1_type){
            printf("\nerror on line %d: Expected 'do-while' condition to be 'u1', got '", u24_unpack(expression.line));
            type_print(types[condition_type]);
            printf("'\n");
            return 1;
        }

        (u0) emit_end_basicblock_jump_conditional(then_block, continuation_block);
    }

    // Continuation block
    emit_start_basicblock_landing(continuation_block, pushed);
    emit_settings.break_basicblock_context = old_break_context;
    emit_settings.continue_basicblock_context = old_continue_context;
    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    return 0;
}

static ErrorCode emit_do_while_tape(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_didnt_continue_cell = emit_context.didnt_continue_cell;

    enter_maybe_breakable_continuable_loop_tape(expression, 2);

    u32 starting_cell_index = emit_context.current_cell_index;

    // Set 'condition' cell to true
    printf("[-]+");

    // While 'condition' cell
    printf("[");
    emit_reset_didnt_continue();
    
    // Emit 'do-while' body
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, false) != 0){
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
    
    exit_maybe_breakable_continuable_loop();

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
}

static ErrorCode emit_for(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_for_stack(expression);
    } else {
        return emit_for_tape(expression);
    }
}

static ErrorCode emit_for_stack(Expression expression){
    u1 was_breakable = emit_context.can_break;
    u1 was_continuable = emit_context.can_continue;
    JumpContext old_break_context = emit_settings.break_basicblock_context;
    JumpContext old_continue_context = emit_settings.continue_basicblock_context;

    u32 num_pre = operands[expression.ops];
    u32 num_post = operands[expression.ops + 2];
    u32 num_inside = operands[expression.ops + 3];

    u32 pre_start_statement = emit_context.current_statement + 1;
    u32 post_start_statement = pre_start_statement + num_pre;
    u32 inside_start_statement = post_start_statement + num_post;

    u32 pre_starting_cell_index = emit_context.current_cell_index;

    u32 condition_block = emit_settings.next_basicblock_id++;
    u32 body_block = emit_settings.next_basicblock_id++;
    u32 increment_block = emit_settings.next_basicblock_id++;
    u32 continuation_block = emit_settings.next_basicblock_id++;

    emit_context.can_break = can_loop_break(emit_context.current_statement);
    emit_context.can_continue = can_loop_continue(emit_context.current_statement);

    // Emit pre-statements
    if(emit_body(pre_start_statement, pre_start_statement + num_pre, false) != 0){
        return 1;
    }
    if(!emit_settings.in_basicblock){
        emit_settings.break_basicblock_context = old_break_context;
        emit_settings.continue_basicblock_context = old_continue_context;
        emit_context.can_break = was_breakable;
        emit_context.can_continue = was_continuable;
        return 0;
    }

    u32 pre_num_cells = emit_context.current_cell_index - pre_starting_cell_index;
    u32 pushed = emit_end_basicblock_jump(condition_block);

    if(emit_context.can_break){
        emit_settings.break_basicblock_context = (JumpContext){
            .basicblock_id = continuation_block,
            .num_cells_input = pushed,
        };
    }

    if(emit_context.can_continue){
        emit_settings.continue_basicblock_context = (JumpContext){
            .basicblock_id = increment_block,
            .num_cells_input = pushed,
        };
    }
    
    emit_start_basicblock_landing(condition_block, pushed);
    // Evaluate condition
    u32 condition_type = expression_emit(expressions[operands[expression.ops + 1]]);
    if(condition_type >= TYPES_CAPACITY) return 1;

    if(condition_type != u1_type){
        printf("\nerror on line %d: Expected 'for' condition to be 'u1', got '", u24_unpack(expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return 1;
    }

    u32 after_condition_pushed = emit_end_basicblock_jump_conditional(body_block, continuation_block);
    if(after_condition_pushed != pushed){
        printf("\ninternal error: emit_for_stack() got bad push amount for jumping to body/continuation blocks (%d vs %d)\n", after_condition_pushed, pushed);

        if(emit_context.function < FUNCTIONS_CAPACITY){
            printf("\n  Inside of function: ");
            print_aux_cstr(functions[emit_context.function].name);
            printf("\n");
        }
        return 1;
    }

    emit_start_basicblock_landing(body_block, pushed);
    // Emit 'for' body
    if(emit_body_scoped(inside_start_statement, inside_start_statement + num_inside, false) != 0){
        return 1;
    }
    if(emit_settings.in_basicblock){
        emit_end_basicblock_jump_compatible(increment_block, pushed);
    } else {
        emit_settings.break_basicblock_context = old_break_context;
        emit_settings.continue_basicblock_context = old_continue_context;
        emit_context.can_break = was_breakable;
        emit_context.can_continue = was_continuable;
        return 0;
    }

    emit_start_basicblock_landing(increment_block, pushed);
    // Emit post-statements
    if(emit_body_scoped(post_start_statement, post_start_statement + num_post, false) != 0){
        return 1;
    }
    if(emit_settings.in_basicblock){
        emit_end_basicblock_jump_compatible(condition_block, pushed);
    } else {
        emit_settings.break_basicblock_context = old_break_context;
        emit_settings.continue_basicblock_context = old_continue_context;
        emit_context.can_break = was_breakable;
        emit_context.can_continue = was_continuable;
        return 0;
    }

    emit_start_basicblock_landing(continuation_block, pushed);

    // Deallocate pre-statement variables
    if(pre_num_cells != 0){
        printf("%d<", pre_num_cells);
        emit_context.current_cell_index -= pre_num_cells;
    }

    emit_settings.break_basicblock_context = old_break_context;
    emit_settings.continue_basicblock_context = old_continue_context;
    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    return 0;
}

static ErrorCode emit_for_tape(Expression expression){
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

    u32 for_statement_index = emit_context.current_statement;

    // Emit pre-statements
    if(emit_body(pre_start_statement, pre_start_statement + num_pre, false) != 0){
        return 1;
    }

    emit_context.current_statement = for_statement_index;
    enter_maybe_breakable_continuable_loop_tape(expression, 4);
    emit_context.current_statement = pre_start_statement + num_pre;

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
    if(emit_body(inside_start_statement, inside_start_statement + num_inside, false) != 0){
        return 1;
    }

    // Emit post-statements
    u32 post_statements_closes_needed_waterline = num_closes_needed;

    if( emit_break_check() != 0
     || emit_early_return_check() != 0
     || emit_body(post_start_statement, post_start_statement + num_post, false) != 0
     || emit_close_checks_until(post_statements_closes_needed_waterline) != 0
    ){
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
    (u0) expression_emit(expressions[operands[expression.ops + 1]]);
    emit_post_loop_condition_break_check();
    emit_post_loop_condition_early_return_check();

    // Go to 'condition' cell
    printf("<");
    emit_context.current_cell_index--;

    // End while
    printf("]");

    exit_maybe_breakable_continuable_loop();

    // Deallocate pre-statement variables
    if(emit_context.current_cell_index > pre_starting_cell_index){
        printf("%d<", emit_context.current_cell_index - pre_starting_cell_index);
        emit_context.current_cell_index = pre_starting_cell_index;
    }

    emit_context.can_break = was_breakable;
    emit_context.can_continue = was_continuable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.didnt_continue_cell = old_didnt_continue_cell;

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
}

static ErrorCode emit_conditionless_block(Expression expression){
    // Emit body
    if(emit_body_scoped(emit_context.current_statement + 1, emit_context.current_statement + expression.ops + 1, false) != 0){
        return 1;
    }

    return 0;
}

static ErrorCode emit_switch(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_switch_stack(expression);
    } else {
        return emit_switch_tape(expression);
    }
}

static u32 emit_switch_eval_value(Expression switch_expression){
    // Returns size of value or -1 on error

    u32 condition_type = expression_emit(expressions[operands[switch_expression.ops]]);
    if(condition_type >= TYPES_CAPACITY) return -1;

    u1 ok = false;

    if(condition_type == u8_type){
        ok = true;
    } else {
        u32 found_enum = find_enum_from_type(condition_type);
        
        if(found_enum <= TYPEDEFS_CAPACITY){
            TypeDef def = typedefs[found_enum];

            if(def.kind == TYPEDEF_ENUM && def.computed_size == 1){
                ok = true;
            }
        }
    }

    if(!ok){
        printf("\nerror on line %d: Expected 'switch' value to be 'u8', got '", u24_unpack(switch_expression.line));
        type_print(types[condition_type]);
        printf("'\n");
        return -1;
    }

    u32 condition_type_size = type_sizeof_or_max(condition_type, switch_expression.line);
    if(condition_type_size == -1) return -1;

    emit_context.switch_value_type = condition_type;
    emit_context.switch_value_type_cached_size = condition_type_size;
    return condition_type_size;
}

typedef struct {
    u32 num_cases;
    u1 has_default;
} SwitchInfo;

static SwitchInfo get_switch_case_info(Expression switch_expression){
    SwitchInfo info;

    u32 length = operands[switch_expression.ops + 1];
    u32 start_statement = emit_context.current_statement + 1;
    u32 stop_statement = start_statement + length;

    for(u32 i = start_statement; i < stop_statement; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_NONE:
        case EXPRESSION_RETURN:
        case EXPRESSION_DECLARE:
        case EXPRESSION_PRINT_LITERAL:
        case EXPRESSION_PRINT_ARRAY:
        case EXPRESSION_PRINTF:
        case EXPRESSION_MEMCMP:
        case EXPRESSION_MEMCPY:
        case EXPRESSION_CALL:
        case EXPRESSION_IMPLEMENT_PUT:
        case EXPRESSION_IMPLEMENT_PRINTU1:
        case EXPRESSION_IMPLEMENT_PRINTU8:
        case EXPRESSION_IMPLEMENT_GET:
        case EXPRESSION_IMPLEMENT_READU8:
        case EXPRESSION_U1:
        case EXPRESSION_U8:
        case EXPRESSION_U16:
        case EXPRESSION_U24:
        case EXPRESSION_U32:
        case EXPRESSION_INT:
        case EXPRESSION_VARIABLE:
        case EXPRESSION_CAST:
        case EXPRESSION_ASSIGN:
        case EXPRESSION_ADD:
        case EXPRESSION_SUBTRACT:
        case EXPRESSION_MULTIPLY:
        case EXPRESSION_DIVIDE:
        case EXPRESSION_MOD:
        case EXPRESSION_EQUALS:
        case EXPRESSION_NOT_EQUALS:
        case EXPRESSION_LESS_THAN:
        case EXPRESSION_GREATER_THAN:
        case EXPRESSION_LESS_THAN_OR_EQUAL:
        case EXPRESSION_GREATER_THAN_OR_EQUAL:
        case EXPRESSION_LSHIFT:
        case EXPRESSION_RSHIFT:
        case EXPRESSION_AND:
        case EXPRESSION_OR:
        case EXPRESSION_BIT_AND:
        case EXPRESSION_BIT_OR:
        case EXPRESSION_BIT_XOR:
        case EXPRESSION_NEGATE:
        case EXPRESSION_NOT:
        case EXPRESSION_BIT_COMPLEMENT:
        case EXPRESSION_INDEX:
        case EXPRESSION_PRE_INCREMENT:
        case EXPRESSION_PRE_DECREMENT:
        case EXPRESSION_POST_INCREMENT:
        case EXPRESSION_POST_DECREMENT:
        case EXPRESSION_NO_RESULT_INCREMENT:
        case EXPRESSION_NO_RESULT_DECREMENT:
        case EXPRESSION_TERNARY:
        case EXPRESSION_MEMBER:
        case EXPRESSION_STRING:
        case EXPRESSION_BREAK:
        case EXPRESSION_CONTINUE:
        case EXPRESSION_SIZEOF_TYPE:
        case EXPRESSION_SIZEOF_TYPE_U8:
        case EXPRESSION_SIZEOF_TYPE_U16:
        case EXPRESSION_SIZEOF_TYPE_U24:
        case EXPRESSION_SIZEOF_TYPE_U32:
        case EXPRESSION_SIZEOF_VALUE:
        case EXPRESSION_SIZEOF_VALUE_U8:
        case EXPRESSION_SIZEOF_VALUE_U16:
        case EXPRESSION_SIZEOF_VALUE_U24:
        case EXPRESSION_SIZEOF_VALUE_U32:
        case EXPRESSION_ARRAY_INITIALIZER:
        case EXPRESSION_STRUCT_INITIALIZER:
        case EXPRESSION_FIELD_INITIALIZER:
        case EXPRESSION_ENUM_VARIANT:
        case EXPRESSION_PANICLOOP:
            break;
        case EXPRESSION_IF:
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
        case EXPRESSION_SWITCH:
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            i += operands[expression.ops + 1] + operands[expression.ops + 2];
            break;
        case EXPRESSION_CONDITIONLESS_BLOCK:
            i += expression.ops;
            break;
        case EXPRESSION_FOR:
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        case EXPRESSION_CASE:
            if(operands[expression.ops] == EXPRESSIONS_CAPACITY){
                info.has_default = true;
            } else {
                info.num_cases++;
            }
            break;
        }
    }

    return info;
}

static ErrorCode emit_switch_stack(Expression expression){
    /*
    EXAMPLE SOURCE CODE:

        switch(value){
        case 100:
            a();
            break;
        case 75:
            b();
        default:
            c();
        }

    Each case testing gets it's own basicblock
    Each case gets it's own entry basicblock
    */

    SwitchInfo info = get_switch_case_info(expression);
    u32 num_cases = info.num_cases;
    u1 has_default_case = info.has_default;

    // Reserve Basicblock IDs for each test case
    u32 first_case_test_basicblock_id = emit_settings.next_basicblock_id;
    emit_settings.next_basicblock_id += num_cases;

    // Reserve Basicblock ID for default case (if it exists)
    u32 default_block = emit_settings.next_basicblock_id++;
    u32 continuation_block;

    if(has_default_case){
        continuation_block = emit_settings.next_basicblock_id++;
    } else {
        continuation_block = default_block;
    }

    // Reserve Basicblock IDs for each case body
    u32 first_case_body_basicblock_id = emit_settings.next_basicblock_id;
    emit_settings.next_basicblock_id += num_cases;

    u1 was_breakable = emit_context.can_break;
    u32 old_switch_value_type = emit_context.switch_value_type;
    u32 old_switch_value_type_cached_size = emit_context.switch_value_type_cached_size;
    SwitchContext old_switch_context = emit_settings.switch_basicblock_context;
    JumpContext old_break_basicblock_context = emit_settings.break_basicblock_context;

    // Evaluate condition
    u32 condition_type_size = emit_switch_eval_value(expression);
    if(condition_type_size == -1){
        return 1;
    }

    u32 pushed = emit_end_basicblock_jump(first_case_test_basicblock_id);

    // Setup how-to-break information
    emit_context.can_break = true;
    emit_settings.switch_basicblock_context = (SwitchContext){
        .num_cases = num_cases,
        .has_default_case = has_default_case,
        .pushed = pushed,
        .first_case_test_basicblock_id = first_case_test_basicblock_id,
        .first_case_body_basicblock_id = first_case_body_basicblock_id,
        .next_case = (u32) 0,
        .default_basicblock_id = default_block,
        .continuation_basicblock_id = continuation_block,
    };
    emit_settings.break_basicblock_context = (JumpContext){
        .basicblock_id = continuation_block,
        .num_cells_input = pushed,
    };

    // Emit 'switch' body
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, true) != 0){
        return 1;
    }

    if(emit_settings.in_basicblock){
        u32 extra = emit_context.current_cell_index - emit_settings.switch_basicblock_context.pushed - emit_settings.stack_driver_position;

        // Discard extra
        if(extra != 0){
            printf("%d<", extra);
            emit_context.current_cell_index -= extra;
        }

        emit_end_basicblock_jump_compatible(continuation_block, pushed);
    }

    emit_start_basicblock_landing(continuation_block, pushed);

    // Discard switch value
    printf("%d<", condition_type_size);
    emit_context.current_cell_index -= condition_type_size;

    emit_context.can_break = was_breakable;
    emit_context.switch_value_type = old_switch_value_type;
    emit_context.switch_value_type_cached_size = old_switch_value_type_cached_size;
    emit_settings.switch_basicblock_context = old_switch_context;
    emit_settings.break_basicblock_context = old_break_basicblock_context;
    return 0;
}

static ErrorCode emit_switch_tape(Expression expression){
    // Switches are implemented by leveraging existing break/continue/early-return infrastructure.
    
    /*
    EXAMPLE SOURCE CODE:

        switch(value){
        case 100:
            a();
            break;
        case 75:
            b();
        default:
            c();
        }

    PSEUDO CODE RESULT:

        {
            u8 value = value;
            u1 didnt_break = true;
            u1 fell_through = false;

            if(fell_through || value == 100){
                a();
                didnt_break = false;
            }

            if(didnt_break && didnt_continue && didnt_return_early){
                if(fell_through || value == 75){
                    b();
                    fell_through = true;
                }

                // NOTE: In practice these checks are omitted when previous code cannot cause them.
                if(didnt_break && didnt_continue && didnt_return_early){
                    // (default case)
                    c();
                }
            }
        }
    */

    u1 was_breakable = emit_context.can_break;
    u32 old_didnt_break_cell = emit_context.didnt_break_cell;
    u32 old_switch_value_type = emit_context.switch_value_type;
    u32 old_switch_value_type_cached_size = emit_context.switch_value_type_cached_size;
    u32 old_switch_start_cell_index = emit_context.switch_start_cell_index;
    u32 old_fell_through_cell = emit_context.fell_through_cell;

    enter_switch();

    u32 starting_cell_index = emit_context.current_cell_index;
    emit_context.switch_start_cell_index = starting_cell_index;
    
    // Evaluate condition
    u32 condition_type_size = emit_switch_eval_value(expression);
    if(condition_type_size == -1){
        return 1;
    }

    // Emit 'switch' body
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + operands[expression.ops + 1] + 1, true) != 0){
        return 1;
    }

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    exit_switch();

    emit_context.can_break = was_breakable;
    emit_context.didnt_break_cell = old_didnt_break_cell;
    emit_context.switch_value_type = old_switch_value_type;
    emit_context.switch_value_type_cached_size = old_switch_value_type_cached_size;
    emit_context.switch_start_cell_index = old_switch_start_cell_index;
    emit_context.fell_through_cell = old_fell_through_cell;

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
}

static ErrorCode emit_case(Expression expression){
    if(emit_context.in_recursive_function){
        return emit_case_stack(expression);
    } else {
        return emit_case_tape(expression);
    }
}

static ErrorCode emit_case_value_test(u32 value_expression){
    Expression expression = expressions[value_expression];

    // Evaluate test value
    u32 test_type = expression_emit(expression);
    if(test_type >= TYPES_CAPACITY) return 1;

    if(test_type != emit_context.switch_value_type){
        printf("\nerror on line %d: Expected 'case' value to be '", u24_unpack(expression.line));
        type_print(types[emit_context.switch_value_type]);
        printf(", got '");
        type_print(types[test_type]);
        printf("'\n");
        return 1;
    }

    // Compare test value to switch value
    u1 ok = false;

    if(emit_context.switch_value_type == u8_type){
        emit_eq_u8();
        ok = true;
    } else {
        u32 found_enum = find_enum_from_type(emit_context.switch_value_type);
        
        if(found_enum <= TYPEDEFS_CAPACITY){
            TypeDef def = typedefs[found_enum];

            if(def.kind == TYPEDEF_ENUM && def.computed_size == 1){
                emit_eq_u8();
                ok = true;
            }
        }
    }

    if(!ok){
        printf("\nerror on line %d: Unsupported 'case' type '", u24_unpack(expression.line));
        type_print(types[test_type]);
        printf("'\n");
        return 1;
    }

    return 0;
}

static ErrorCode emit_case_stack(Expression case_expression){
    u32 value_expression = operands[case_expression.ops];
    u32 test_block;
    u32 body_block;
    u32 else_block;

    if(value_expression == EXPRESSIONS_CAPACITY){
        // Default case
        test_block = -1;
        else_block = -1;
        body_block = emit_settings.switch_basicblock_context.default_basicblock_id;
    } else {
        u32 case_i = emit_settings.switch_basicblock_context.next_case++;
        test_block = emit_settings.switch_basicblock_context.first_case_test_basicblock_id + case_i;
        body_block = emit_settings.switch_basicblock_context.first_case_body_basicblock_id + case_i;
        else_block = test_block + 1;
    }

    // Fall through from previous case body if still open
    if(emit_settings.in_basicblock){
        u32 extra = emit_context.current_cell_index - emit_settings.switch_basicblock_context.pushed - emit_settings.stack_driver_position;

        // Discard extra
        if(extra != 0){
            printf("%d<", extra);
            emit_context.current_cell_index -= extra;
        }

        // Fall though to next block
        emit_end_basicblock_jump_compatible(body_block, emit_settings.switch_basicblock_context.pushed);
    }

    // Emit test value
    if(value_expression != EXPRESSIONS_CAPACITY){
        emit_start_basicblock_landing(test_block, emit_settings.switch_basicblock_context.pushed);

        // Dupe switch value
        copy_cells_static(emit_context.current_cell_index - emit_context.switch_value_type_cached_size, emit_context.switch_value_type_cached_size);

        if(emit_case_value_test(value_expression) != 0){
            return 1;
        }

        (u0) emit_end_basicblock_jump_conditional(body_block, else_block);
    }

    emit_start_basicblock_landing(body_block, emit_settings.switch_basicblock_context.pushed);

    // Emit 'case' body
    u32 len = operands[case_expression.ops + 1];
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + len + 1, false) != 0){
        return 1;
    }

    // Don't end basicblock unless ended by user
    return 0;
}

static ErrorCode emit_case_tape(Expression expression){
    // Emits code for case statements

    if(emit_context.switch_value_type >= TYPES_CAPACITY){
        expression_print_cannot_use_case_here_error(expression);
        return 1;
    }

    u32 starting_cell_index = emit_context.current_cell_index;
    u32 value_expression = operands[expression.ops];

    if(value_expression != EXPRESSIONS_CAPACITY){
        emit_pre_case_fallthrough_check();

        // Dupe switch value
        copy_cells_static(emit_context.switch_start_cell_index, emit_context.switch_value_type_cached_size);

        if(emit_case_value_test(value_expression) != 0){
            return 1;
        }

        emit_post_case_fallthrough_check();

        // Go to condition
        printf("<");
        emit_context.current_cell_index--;

        // If condition
        printf("[");
    }

    // Emit 'case' body
    u32 len = operands[expression.ops + 1];
    if(emit_body(emit_context.current_statement + 1, emit_context.current_statement + len + 1, false) != 0){
        return 1;
    }

    // Deallocate variables
    if(emit_context.current_cell_index > starting_cell_index){
        printf("%d<", emit_context.current_cell_index - starting_cell_index);
        emit_context.current_cell_index = starting_cell_index;
    }

    if(value_expression != EXPRESSIONS_CAPACITY){
        ExpressionKind termination = expressions[statements[emit_context.current_statement]].kind;

        if(len == 0 || !(termination == EXPRESSION_BREAK || termination == EXPRESSION_CONTINUE || termination == EXPRESSION_RETURN)){
            // Fall through
            u32 offset = emit_context.current_cell_index - emit_context.fell_through_cell;
            printf("%d<[-]+%d>", offset, offset);
        }

        // End if
        printf("[-]]");
    }

    return (ErrorCode) (emit_break_check() != 0 || emit_continue_check() != 0 || emit_early_return_check() != 0);
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
        case EXPRESSION_NONE:
        case EXPRESSION_DECLARE:
        case EXPRESSION_PRINT_LITERAL:
        case EXPRESSION_PRINT_ARRAY:
        case EXPRESSION_PRINTF:
        case EXPRESSION_MEMCMP:
        case EXPRESSION_MEMCPY:
        case EXPRESSION_CALL:
        case EXPRESSION_IMPLEMENT_PUT:
        case EXPRESSION_IMPLEMENT_PRINTU1:
        case EXPRESSION_IMPLEMENT_PRINTU8:
        case EXPRESSION_IMPLEMENT_GET:
        case EXPRESSION_IMPLEMENT_READU8:
        case EXPRESSION_U1:
        case EXPRESSION_U8:
        case EXPRESSION_U16:
        case EXPRESSION_U24:
        case EXPRESSION_U32:
        case EXPRESSION_INT:
        case EXPRESSION_VARIABLE:
        case EXPRESSION_CAST:
        case EXPRESSION_ASSIGN:
        case EXPRESSION_ADD:
        case EXPRESSION_SUBTRACT:
        case EXPRESSION_MULTIPLY:
        case EXPRESSION_DIVIDE:
        case EXPRESSION_MOD:
        case EXPRESSION_EQUALS:
        case EXPRESSION_NOT_EQUALS:
        case EXPRESSION_LESS_THAN:
        case EXPRESSION_GREATER_THAN:
        case EXPRESSION_LESS_THAN_OR_EQUAL:
        case EXPRESSION_GREATER_THAN_OR_EQUAL:
        case EXPRESSION_LSHIFT:
        case EXPRESSION_RSHIFT:
        case EXPRESSION_AND:
        case EXPRESSION_OR:
        case EXPRESSION_BIT_AND:
        case EXPRESSION_BIT_OR:
        case EXPRESSION_BIT_XOR:
        case EXPRESSION_NEGATE:
        case EXPRESSION_NOT:
        case EXPRESSION_BIT_COMPLEMENT:
        case EXPRESSION_INDEX:
        case EXPRESSION_PRE_INCREMENT:
        case EXPRESSION_PRE_DECREMENT:
        case EXPRESSION_POST_INCREMENT:
        case EXPRESSION_POST_DECREMENT:
        case EXPRESSION_NO_RESULT_INCREMENT:
        case EXPRESSION_NO_RESULT_DECREMENT:
        case EXPRESSION_TERNARY:
        case EXPRESSION_MEMBER:
        case EXPRESSION_STRING:
        case EXPRESSION_BREAK:
        case EXPRESSION_CONTINUE:
        case EXPRESSION_SIZEOF_TYPE:
        case EXPRESSION_SIZEOF_TYPE_U8:
        case EXPRESSION_SIZEOF_TYPE_U16:
        case EXPRESSION_SIZEOF_TYPE_U24:
        case EXPRESSION_SIZEOF_TYPE_U32:
        case EXPRESSION_SIZEOF_VALUE:
        case EXPRESSION_SIZEOF_VALUE_U8:
        case EXPRESSION_SIZEOF_VALUE_U16:
        case EXPRESSION_SIZEOF_VALUE_U24:
        case EXPRESSION_SIZEOF_VALUE_U32:
        case EXPRESSION_CASE:
        case EXPRESSION_ARRAY_INITIALIZER:
        case EXPRESSION_STRUCT_INITIALIZER:
        case EXPRESSION_FIELD_INITIALIZER:
        case EXPRESSION_ENUM_VARIANT:
        case EXPRESSION_PANICLOOP:
            break;

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
        case EXPRESSION_CONDITIONLESS_BLOCK:
            if(has_return_in_region(i + 1, i + expression.ops + 1)) return true;
            i += expression.ops + 1;
            break;
        case EXPRESSION_FOR: {
                u32 num_pre = operands[expression.ops];
                u32 num_post = operands[expression.ops + 2];
                u32 len = operands[expression.ops + 3];

                if(has_return_in_region(i + 1, i + num_pre + num_post + len + 1)) return true;
                i += num_pre + num_post + len;
            }
            break;
        case EXPRESSION_SWITCH:
            if(has_return_in_region(i + 1, i + operands[expression.ops + 1] + 1)) return true;
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
    case EXPRESSION_FOR: {
            u32 num_pre = operands[container.ops];
            u32 num_post = operands[container.ops + 2];
            u32 num_inside = operands[container.ops + 3];

            u32 pre_start_statement = statement_index + 1;
            u32 post_start_statement = pre_start_statement + num_pre;
            u32 inside_start_statement = post_start_statement + num_post;

            return
                can_statements_break_current_level(inside_start_statement, inside_start_statement + num_inside)
             || can_statements_break_current_level(post_start_statement, post_start_statement + num_post);
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
    case EXPRESSION_FOR: {
            u32 num_pre = operands[container.ops];
            u32 num_post = operands[container.ops + 2];
            u32 num_inside = operands[container.ops + 3];

            u32 pre_start_statement = statement_index + 1;
            u32 post_start_statement = pre_start_statement + num_pre;
            u32 inside_start_statement = post_start_statement + num_post;

            return
                can_statements_continue_current_level(inside_start_statement, inside_start_statement + num_inside)
             || can_statements_continue_current_level(post_start_statement, post_start_statement + num_post);
        }
        break;
    default:
        return can_statement_continue_current_level(statement_index);
    }
}

static ErrorCode emit_globals_initialization(){
    u32 global_variable_cell_offset = 0;
    for(u32 global_i = 0; global_i < num_globals; global_i++){
        Global global = globals[global_i];

        u32 size = type_sizeof_or_max(global.type, global.line);
        if(size == -1) return 1;

        if(global.initializer < EXPRESSIONS_CAPACITY){
            u32 type = expression_emit(expressions[global.initializer]);
            if(type >= TYPES_CAPACITY) return 1;

            if(type != global.type){
                if(grow_type(type, global.type, 0) != 0){
                    printf("\nerror on line %d: Cannot assign '", u24_unpack(global.line));
                    type_print(types[type]);
                    printf("' to '");
                    type_print(types[global.type]);
                    printf("'\n");
                    return 1;
                }

                type = global.type;
            }

            printf("<");
            emit_context.current_cell_index--;

            move_cells_static(global_variable_cell_offset, size);
        }

        global_variable_cell_offset += size;
    }

    return 0;
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
        .incomplete_cell = -(u32) 1,
        .didnt_break_cell = -(u32) 1,
        .didnt_continue_cell = -(u32) 1,
        .switch_value_type = TYPES_CAPACITY,
        .switch_value_type_cached_size = -(u32) 1,
        .switch_start_cell_index = -(u32) 1,
        .fell_through_cell = -(u32) 1,
        .can_break = false,
        .can_continue = false,
    };
    
    // Allocate 'incomplete' cell if function can return early
    if(can_early_return){
        printf("[-]+>");
        emit_context.incomplete_cell = emit_context.current_cell_index++;
    }

    if(function_index == emit_settings.main_function_index){
        if(function.return_type != u0_type){
            printf("\nerror on line %d: Main function must have the return type 'u0'\n", u24_unpack(function.line));
            return 1;
        }

        if(emit_globals_initialization() != 0){
            return 1;
        }
    }

    if(emit_body(function.begin + (u32) function.arity, function.begin + function.num_stmts, false) != 0){
        return 1;
    }

    if(emit_context.current_cell_index > start_function_cell_index){
        printf("%d<", emit_context.current_cell_index - start_function_cell_index);
    }

    emit_context = old_emit_context;
    emit_context.current_cell_index = start_function_cell_index;
    return 0;
}

