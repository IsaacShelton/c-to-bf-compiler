
#include <stdio.h>
#include "../include/variable_find.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/type_emit.h"
#include "../include/expression_emit.h"
#include "../include/emit_context.h"

typedef struct {
    u32 self_statement;
    u32 start_statement;
    u32 stop_statement;
    u32 secondary_start_statement;
    u32 secondary_stop_statement;
} Container;

static Container get_parent_container(u32 statement_index){
    // Returns Conainer with start_statement >= STATEMENTS_CAPACITY when not parent container exists

    if(statement_index >= STATEMENTS_CAPACITY){
        // No parent container exists
        return (Container){
            .self_statement = STATEMENTS_CAPACITY,
            .start_statement = STATEMENTS_CAPACITY,
            .stop_statement = STATEMENTS_CAPACITY,
            .secondary_start_statement = STATEMENTS_CAPACITY,
            .secondary_stop_statement = STATEMENTS_CAPACITY,
        };
    }

    Function function = functions[emit_context.function];

    u1 allow_case = expressions[statements[statement_index]].kind != EXPRESSION_CASE;

    for(u32 i = statement_index - 1; i != function.begin - 1; i--){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_IF:
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE: {
                u32 num_statements = operands[expression.ops + 1];

                if(i + num_statements >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + 1,
                        .stop_statement = i + num_statements + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }
            }
            break;
        case EXPRESSION_SWITCH: {
                u32 num_statements = operands[expression.ops + 1];

                if(i + num_statements >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + 1,
                        .stop_statement = i + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }
            }
            break;
        case EXPRESSION_IF_ELSE: {
                u32 num_then = operands[expression.ops + 1];
                u32 num_else = operands[expression.ops + 2];

                if(i + num_then >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + 1,
                        .stop_statement = i + num_then + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }

                if(i + num_then + num_else >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + num_then + 1,
                        .stop_statement = i + num_then + num_else + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }
            }
            break;
        case EXPRESSION_FOR: {
                u32 num_pre = operands[expression.ops];
                u32 num_post = operands[expression.ops + 2];
                u32 num_inside = operands[expression.ops + 3];

                // Check if in pre-statements
                if(i + num_pre >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + 1,
                        .stop_statement = i + num_pre + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }

                // Check if in post-statements
                if(i + num_pre >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + num_pre + 1,
                        .stop_statement = i + num_pre + num_post + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }

                // Check if in main body
                if(i + num_pre + num_post + num_inside >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + num_pre + num_post + 1,
                        .stop_statement = i + num_pre + num_post + num_inside + 1,
                        .secondary_start_statement = i + 1,
                        .secondary_stop_statement = i + num_pre + 1,
                    };
                }
            }
            break;
        case EXPRESSION_CASE: {
                u32 num_statements = operands[expression.ops + 1];

                if(allow_case && i + num_statements >= statement_index){
                    return (Container){
                        .self_statement = i,
                        .start_statement = i + 1,
                        .stop_statement = i + num_statements + 1,
                        .secondary_start_statement = STATEMENTS_CAPACITY,
                        .secondary_stop_statement = STATEMENTS_CAPACITY,
                    };
                }
            }
            break;
        }
    }

    // Function "container"
    return (Container){
        .self_statement = STATEMENTS_CAPACITY,
        .start_statement = function.begin,
        .stop_statement = function.begin + function.num_stmts + 1,
        .secondary_start_statement = STATEMENTS_CAPACITY,
        .secondary_stop_statement = STATEMENTS_CAPACITY,
    };
}

u32 find_declaration(u32 start_statement, u32 stop_statement, u32 name){
    // Returns STATEMENTS_CAPACITY when cannot find

    for(u32 i = start_statement; i < stop_statement; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_IF:
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
        case EXPRESSION_SWITCH:
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            i += operands[expression.ops + 1] + operands[expression.ops + 2];
            break;
        case EXPRESSION_FOR:
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        case EXPRESSION_DECLARE:
            if(aux_cstr_equals(operands[expression.ops + 1], name)){
                return i;
            }
            break;
        }
    }

    return STATEMENTS_CAPACITY;
}

typedef struct {
    u32 delta_i;
    u32 delta_depth;
    u32 delta_offset;
} HoneInfo;

static HoneInfo hone_switch_case_or_skip(u32 current_statement, u32 target_statement, u32 num_statements){
    for(u32 i = current_statement + 1; i < current_statement + num_statements + 1; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_IF:
        case EXPRESSION_WHILE:
        case EXPRESSION_DO_WHILE:
        case EXPRESSION_SWITCH:
            i += operands[expression.ops + 1];
            break;
        case EXPRESSION_IF_ELSE:
            i += operands[expression.ops + 1] + operands[expression.ops + 2];
        case EXPRESSION_FOR:
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        case EXPRESSION_CASE: {
                u32 num_case_statements = operands[expression.ops + 1];

                if(target_statement <= i + num_case_statements){
                    return (HoneInfo){ .delta_i = i - (current_statement + 1), .delta_depth = 1, .delta_offset = 2 };
                }
            }
            break;
        }
    }

    return (HoneInfo){ .delta_i = num_statements, .delta_depth = 0, .delta_offset = 0 };
}

HoneInfo hone_statement(u32 current_statement, u32 target_statement);

static HoneInfo hone_for_body_or_skip(u32 current_statement, u32 target_statement, u32 num_pre, u32 num_post, u32 len, u32 inner_variable_offset){
    u32 pre_offset = 0;

    for(u32 i = current_statement + 1; i < current_statement + 1 + num_pre; i++){
        Expression expression = expressions[statements[i]];

        if(expression.kind == EXPRESSION_DECLARE){
            u32 type_size = type_sizeof_or_max(operands[expression.ops]);

            if(type_size != -1){
                pre_offset += type_size;
            }
        } else {
            // Skip over constructs
            HoneInfo hone_info = hone_statement(i, target_statement);
            i += hone_info.delta_i;
            pre_offset += hone_info.delta_offset;
        }
    }

    return (HoneInfo){ .delta_i = num_pre + num_post, .delta_depth = 1, .delta_offset = pre_offset + inner_variable_offset };
}

HoneInfo hone_statement(u32 current_statement, u32 target_statement){
    Expression expression = expressions[statements[current_statement]];

    switch(expression.kind){
    case EXPRESSION_IF: {
            u32 num_statements = operands[expression.ops + 1];

            if(target_statement <= current_statement + num_statements){
                return (HoneInfo){ .delta_i = 0, .delta_depth = 1, .delta_offset = 0 };
            } else {
                return (HoneInfo){ .delta_i = num_statements, .delta_depth = 0, .delta_offset = 0 };
            }
        }
        break;
    case EXPRESSION_IF_ELSE: {
            u32 num_then = operands[expression.ops + 1];
            u32 num_else = operands[expression.ops + 2];

            if(target_statement <= current_statement + num_then){
                return (HoneInfo){ .delta_i = 0, .delta_depth = 1, .delta_offset = 0 };
            } else if(target_statement <= current_statement + num_then + num_else){
                return (HoneInfo){ .delta_i = num_then, .delta_depth = 1, .delta_offset = 0 };
            } else {
                return (HoneInfo){ .delta_i = num_then + num_else, .delta_depth = 0, .delta_offset = 0 };
            }
        }
        break;
    case EXPRESSION_WHILE:
    case EXPRESSION_DO_WHILE: {
            u32 num_statements = operands[expression.ops + 1];
            u32 inner_variable_offset = operands[expression.ops + 2];

            if(target_statement <= current_statement + num_statements){
                return (HoneInfo){ .delta_i = 0, .delta_depth = 1, .delta_offset = inner_variable_offset };
            } else {
                return (HoneInfo){ .delta_i = num_statements, .delta_depth = 0, .delta_offset = 0 };
            }
        }
        break;
    case EXPRESSION_FOR: {
            u32 num_pre = operands[expression.ops];
            u32 num_post = operands[expression.ops + 2];
            u32 len = operands[expression.ops + 3];
            u32 inner_variable_offset = operands[expression.ops + 4];

            if(target_statement <= current_statement + num_pre){
                return (HoneInfo){ .delta_i = 0, .delta_depth = 1, .delta_offset = 0 };
            } else if(target_statement <= current_statement + num_pre + num_post){
                // NOTE: Should never happen since referencing these variables is not allowed under normal circumstances
                return (HoneInfo){ .delta_i = num_pre, .delta_depth = 1, .delta_offset = inner_variable_offset };
            } else if(target_statement <= current_statement + num_pre + num_post + len){
                return hone_for_body_or_skip(current_statement, target_statement, num_pre, num_post, len, inner_variable_offset);
            } else {
                return (HoneInfo){ .delta_i = num_pre + num_post + len, .delta_depth = 0, .delta_offset = 0 };
            }
        }
        break;
    case EXPRESSION_SWITCH: {
            u32 num_statements = operands[expression.ops + 1];

            // Find which case
            return hone_switch_case_or_skip(current_statement, target_statement, num_statements);
        }
        break;
    }

    return (HoneInfo){ .delta_i = 0, .delta_depth = 0, .delta_offset = 0 };
}

Variable get_variable_location_from_declaration_statement(u32 statement_index){
    Expression expression = expressions[statements[statement_index]];

    u32 depth = 1;
    u32 offset = 0;
    u32 function_begin = functions[emit_context.function].begin;
    u32 function_arity = functions[emit_context.function].arity;

    for(u32 i = function_begin; i < statement_index; i++){
        Expression expression = expressions[statements[i]];

        if(emit_context.can_function_early_return && i == function_begin + function_arity + 1){
            offset++;
        }

        if(expression.kind == EXPRESSION_DECLARE){
            u32 type_size = type_sizeof_or_max(operands[expression.ops]);

            if(type_size != -1){
                offset += type_size;
            }
        } else {
            HoneInfo hone_info = hone_statement(i, statement_index);
            i += hone_info.delta_i;
            depth += hone_info.delta_depth;
            offset += hone_info.delta_offset;
        }
    }

    return (Variable){
        .name = operands[expression.ops + 1],
        .type = operands[expression.ops + 0],
        .defined = true,
        .depth = depth,
        .declaration = statement_index,
        .location = (VariableLocation){
            .on_stack = false,
            .location = emit_context.function_cell_index + offset,
        },
    };
}

static u32 u32_min2(u32 a, u32 b){
    return a <= b ? a : b;
}

Variable variable_find(u32 name){
    if(emit_context.in_recursive_function){
        printf("\nerror: variable_find not yet implemented for recursive functions\n");
        return (Variable){ .defined = false };
    }

    for(
        Container container = get_parent_container(emit_context.current_statement);
        container.start_statement < STATEMENTS_CAPACITY;
        container = get_parent_container(container.self_statement)
    ){
        u32 declaration = find_declaration(container.start_statement, u32_min2(container.stop_statement, emit_context.current_statement), name);

        if(declaration < STATEMENTS_CAPACITY){
            return get_variable_location_from_declaration_statement(declaration);
        }

        if(container.secondary_start_statement < STATEMENTS_CAPACITY){
            declaration = find_declaration(container.secondary_start_statement, u32_min2(container.secondary_stop_statement, emit_context.current_statement), name);

            if(declaration < STATEMENTS_CAPACITY){
                return get_variable_location_from_declaration_statement(declaration);
            }
        }
    }

    // Search global
    // (not implemented yet)

    // Not found
    return (Variable){ .defined = false };
}

