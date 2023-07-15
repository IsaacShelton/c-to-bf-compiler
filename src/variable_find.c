
#include "../include/variable_find.h"
#include "../include/expression.h"
#include "../include/storage.h"

Variable variable_find(int function_beginning, int current_statement, int name){
    Variable variable = (Variable){
        .name = 0,
        .type = 0,
        .depth = 0,
        .defined = false,
        .declaration = 0,
    };

    int depth = 1;

    // Look for local variable
    for(int i = function_beginning; i < current_statement; i++){
        Expression expression = expressions[statements[i]];

        switch(expression.kind){
        case EXPRESSION_DECLARE:
            if(aux_cstr_equals(operands[expression.ops + 1], name) && depth > variable.depth){
                variable = (Variable){
                    .name = name,
                    .type = operands[expression.ops + 0],
                    .defined = true,
                    .depth = depth,
                    .declaration = i,
                };
            }
            break;
        /* otherwise, don't care */
        }
    }

    // If nothing found yet look for global variable
    if(!variable.defined){
        for(int i = 0; i < num_globals; i++){
            if(aux_cstr_equals(globals[i].name, name)){
                return (Variable){
                    .name = name,
                    .type = globals[i].type,
                    .defined = true,
                    .depth = 0,
                    .declaration = i,
                };
            }
        }
    }

    // Return best fit
    return variable;
}

