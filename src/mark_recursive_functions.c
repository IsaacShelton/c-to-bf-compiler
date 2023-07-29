
#include <stdio.h>
#include "../include/mark_recursive_functions.h"
#include "../include/config.h"
#include "../include/storage.h"
#include "../include/function.h"

typedef struct {
    u32 from;
    u32 to;
} Arrow;

#define EXPRESSIONS_TO_PROCESS_CAPACITY MEMORY_MULTIPLIER * 1024

ErrorCode mark_recursive_functions(){
    Arrow arrows[ARROWS_CAPACITY];
    u32 num_arrows = 0;

    u32 expressions_to_process[EXPRESSIONS_TO_PROCESS_CAPACITY];
    u32 num_expressions_to_process = 0;

    // Create arrows for each function call
    for(u32 function_i = 0; function_i < num_functions; function_i++){
        Function function = functions[function_i];

        u32 function_arrows_begin = num_arrows;
        u32 statements_start = function.begin + function.arity;
        u32 statements_end = statements_start + function.num_stmts;
        u32 statement_i = statements_start;

        // Find all calls made in this function
        while(true){
            Expression expression;

            // Get next expression to process
            if(num_expressions_to_process > 0){
                expression = expressions[expressions_to_process[--num_expressions_to_process]];
            } else if(statement_i < statements_end){
                expression = expressions[statements[statement_i++]];
            } else {
                break;
            }

            // Process expression to find all calls made
            switch(expression.kind){
            case EXPRESSION_CALL: {
                    u32 arity = operands[expression.ops + 1];

                    if(num_expressions_to_process + arity >= EXPRESSIONS_TO_PROCESS_CAPACITY){
                        printf("\nout of memory: Exceeded maximum number of expressions being processed during determination of recursive functions\n");
                        return 1;
                    }

                    for(u32 i = 0; i < arity; i++){
                        expressions_to_process[num_expressions_to_process++] = operands[expression.ops + 2 + i];
                    }

                    u32 to = find_function(operands[expression.ops]);

                    if(to < FUNCTIONS_CAPACITY){
                        if(num_arrows + 1 >= ARROWS_CAPACITY){
                            printf("\nout of memory: Exceeded maximum number of arrows during determination of recursive functions\n");
                            return 1;
                        }

                        // Check if arrow already exists
                        u1 already_exists = false;

                        for(u32 i = function_arrows_begin; i < num_arrows; i++){
                            if(arrows[i].to == to){
                                already_exists = true;
                                break;
                            }
                        }

                        // Add arrow if doesn't exist
                        if(!already_exists){
                            arrows[num_arrows++] = (Arrow){
                                .from = function_i,
                                .to = to,
                            };
                        }
                    }
                }
                break;
            case EXPRESSION_ADD:
            case EXPRESSION_SUBTRACT:
                if(num_expressions_to_process + 2 >= EXPRESSIONS_TO_PROCESS_CAPACITY){
                    printf("\nout of memory: Exceeded maximum number of expressions being processed during determination of recursive functions\n");
                    return 1;
                }

                expressions_to_process[num_expressions_to_process++] = operands[expression.ops];
                expressions_to_process[num_expressions_to_process++] = operands[expression.ops + 1];
                break;
            case EXPRESSION_ASSIGN:
                if(num_expressions_to_process + 1 >= EXPRESSIONS_TO_PROCESS_CAPACITY){
                    printf("\nout of memory: Exceeded maximum number of expressions being processed during determination of recursive functions\n");
                    return 1;
                }

                expressions_to_process[num_expressions_to_process++] = operands[expression.ops + 1];
                break;
            case EXPRESSION_RETURN:
                if(num_expressions_to_process + 1 >= EXPRESSIONS_TO_PROCESS_CAPACITY){
                    printf("\nout of memory: Exceeded maximum number of expressions being processed during determination of recursive functions\n");
                    return 1;
                }

                expressions_to_process[num_expressions_to_process++] = expression.ops;
                break;
            default:
                /* ignore */
                break;
            }

            // End while
        }

        // End for
    }

    // Done creating arrows

    /*
    // Try printing arrows
    for(u32 i = 0; i < num_arrows; i++){
        printf("\narrow: %d -> %d\n", arrows[i].from, arrows[i].to);
    }
    */

    u16 outgoing[FUNCTIONS_CAPACITY] = {0};

    // Tally outgoing arrows for each function
    for(u32 i = 0; i < num_arrows; i++){
        outgoing[arrows[i].from]++;
    }

    // Eliminate each function that has zero remaining outgoing arrows (marking them as non-recursive),
    // until we can no longer do so.
    // When we remove a function, all functions that called it will have their
    // Outgoing arrows decremented.
    // The remaining group of functions require recursion, and will retain their 'is_recursive' mark.

    // We assume that all functions that might be recursive as pre-marked as recursive.

    u1 progress = true;

    while(progress){
        progress = false;

        for(u32 function_i = 0; function_i < num_functions; function_i++){
            if(functions[function_i].is_recursive && outgoing[function_i] == 0){
                // Proved this function doesn't need recursion

                progress = true;

                // Mark non-recursive
                functions[function_i].is_recursive = false;

                // Remove edge from functions who use this function
                for(u32 i = 0; i < num_arrows; i++){
                    if(arrows[i].to == function_i){
                        outgoing[arrows[i].from]--;
                    }
                }

                break;
            }
        }
    }

    /*
    // Try print which functions are recursive
    for(u32 i = 0; i < num_functions; i++){
        Function function = functions[i];

        printf("\n");
        print_aux_cstr(function.name);
        printf(" is ");

        if(function.is_recursive){
            printf("recursive\n");
        } else {
            printf("not recursive\n");
        }
    }
    */

    return 0;
}

