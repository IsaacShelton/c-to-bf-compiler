
#include <stdio.h>
#include "../include/variable_find.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/type_emit.h"
#include "../include/expression_emit.h"
#include "../include/emit_context.h"

Variable variable_find(u32 name){
    Variable variable = (Variable){
        .name = 0,
        .type = 0,
        .depth = 0,
        .defined = false,
        .declaration = 0,
    };

    u32 depth = 1;
    u32 offset = 0;

    // Look for local variable
    for(u32 i = emit_context.function_begin_statement; i < emit_context.current_statement; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_DECLARE:
            if(aux_cstr_equals(operands[expression.ops + 1], name) && depth > variable.depth){
                if(emit_context.in_recursive_function){
                    // TODO
                    printf("\nerror: variable_find not fully implemented for recursive functions\n");
                } else {
                    return (Variable){
                        .name = name,
                        .type = operands[expression.ops + 0],
                        .defined = true,
                        .depth = depth,
                        .declaration = i,
                        .location = (VariableLocation){
                            .on_stack = false,
                            .location = emit_context.function_cell_index + offset,
                        },
                    };
                }
            }

            u32 type_size = type_sizeof_or_max(operands[expression.ops]);

            if(type_size != -1){
                offset += type_size;
            }

            break;
        /* otherwise, don't care */
        }
    }

    // If nothing found yet look for global variable
    if(!variable.defined){
        // TODO: Global variable support
        /*
        offset = 0;
        for(u32 i = 0; i < num_globals; i++){
            if(aux_cstr_equals(globals[i].name, name)){
                return (Variable){
                    .name = name,
                    .type = globals[i].type,
                    .defined = true,
                    .depth = 0,
                    .declaration = i,
                    .on_stack = false,
                    .location = (VariableLocation){
                        .on_stack = false,
                        .location = GLOBALS_START + offset,
                    },
                };
            }

            u32 type_size = type_sizeof_or_max(operands[expression.ops]);

            if(type_size != -1){
                offset += type_size;
            }
        }
        */
    }

    // Return best fit
    return variable;
}

