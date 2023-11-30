
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
        case EXPRESSION_NONE:
        case EXPRESSION_RETURN:
        case EXPRESSION_DECLARE:
        case EXPRESSION_PRINT_LITERAL:
        case EXPRESSION_PRINT_ARRAY:
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
        case EXPRESSION_NONE:
        case EXPRESSION_RETURN:
        case EXPRESSION_PRINT_LITERAL:
        case EXPRESSION_PRINT_ARRAY:
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
        case EXPRESSION_NONE:
        case EXPRESSION_RETURN:
        case EXPRESSION_DECLARE:
        case EXPRESSION_PRINT_LITERAL:
        case EXPRESSION_PRINT_ARRAY:
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
        case EXPRESSION_FOR:
            i += operands[expression.ops] + operands[expression.ops + 2] + operands[expression.ops + 3];
            break;
        case EXPRESSION_CASE: {
                u32 num_case_statements = operands[expression.ops + 1];

                if(target_statement <= i + num_case_statements){
                    u32 delta_offset = emit_context.in_recursive_function ? 0 : 2;
                    return (HoneInfo){ .delta_i = i - (current_statement + 1), .delta_depth = 1, .delta_offset = delta_offset };
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

    if(emit_context.in_recursive_function){
        fprintf(stderr, "warning: hone_for_body_or_skip is unimplemented for recursive functions\n");
    }

    for(u32 i = current_statement + 1; i < current_statement + 1 + num_pre; i++){
        Expression expression = expressions[statements[i]];

        if(expression.kind == EXPRESSION_DECLARE){
            u32 type_size = type_sizeof_or_max(operands[expression.ops], expression.line);

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
    case EXPRESSION_NONE:
    case EXPRESSION_RETURN:
    case EXPRESSION_DECLARE:
    case EXPRESSION_PRINT_LITERAL:
    case EXPRESSION_PRINT_ARRAY:
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
            u32 inner_variable_offset = emit_context.in_recursive_function ? 0 : operands[expression.ops + 2];

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
            u32 inner_variable_offset = emit_context.in_recursive_function ? 0 : operands[expression.ops + 4];

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

        if(emit_context.can_function_early_return && i == function_begin + function_arity){
            offset++;
        }

        if(expression.kind == EXPRESSION_DECLARE){
            u32 type_size = type_sizeof_or_max(operands[expression.ops], expression.line);

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
        .location = (VariableLocation){
            .kind = VARIABLE_LOCATION_ON_TAPE,
            .location = emit_context.function_cell_index + offset,
        },
    };
}

static u32 u32_min2(u32 a, u32 b){
    return a <= b ? a : b;
}

Variable variable_find(u32 name){
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
    u32 global_variable_cell_offset = 0;
    for(u32 global_i = 0; global_i < num_globals; global_i++){
        Global global = globals[global_i];

        if(aux_cstr_equals(global.name, name)){
            return (Variable){
                .name = name,
                .type = global.type,
                .defined = true,
                .depth = 0,
                .location = (VariableLocation){
                    .kind = VARIABLE_LOCATION_ON_TAPE,
                    .location = global_variable_cell_offset,
                },
            };
        }

        u32 size = type_sizeof_or_max(global.type, global.line);
        if(size == -1) return (Variable){ .defined = false };

        global_variable_cell_offset += size;
    }

    // Search for enum variant
    for(u32 typedef_i = 0; typedef_i < num_typedefs; typedef_i++){
        TypeDef def = typedefs[typedef_i];

        if(def.kind != TYPEDEF_ENUM) continue;

        for(u32 i = 0; i < def.num_fields; i++){
            Expression variant = expressions[statements[def.begin + i]];
            if(variant.kind != EXPRESSION_ENUM_VARIANT) continue;

            if(aux_cstr_equals(variant.ops, name)){
                u32 enum_type = add_type((Type){
                    .name = def.name,
                    .dimensions = 0
                });

                if(enum_type >= TYPES_CAPACITY){
                    return (Variable){ .defined = false };
                }

                u32 variant_value = i;

                return (Variable){
                    .name = name,
                    .defined = true,
                    .type = enum_type,
                    .depth = 0,
                    .location = (VariableLocation){
                        .kind = VARIABLE_LOCATION_IMMUTABLE,
                        .location = variant_value,
                    }
                };
            }
        }
    }

    // Not found
    return (Variable){ .defined = false };
}

