
#include <stdio.h>
#include "../include/io.h"
#include "../include/lex_string.h"
#include "../include/lex_result.h"
#include "../include/lex_context.h"
#include "../include/storage.h"

LexUnboundedResult lex_string(u8 c){
    // TODO: Refactor to merge buffer-only and past-buffer cases

    tokens[num_tokens - 1].data = num_aux;

    u32 read = 0;
    u1 escape = false;

    for(; read < code_buffer_length; read++){
        u8 string_c = code_buffer[read];

        if(string_c == '\n'){
            lex_line_number++;
        }

        if(escape){
            escape = false;

            if(string_c == 'n'){
                string_c = '\n';
            } else if(c == '\t'){
                string_c = '\t';
            }
        } else if(string_c == '"'){
            break;
        } else if(string_c == '\\'){
            escape = true;
            continue;
        }

        if(num_aux == AUX_CAPACITY){
            printf("Out of memory: Auxiliary memory used up\n");
            return (LexUnboundedResult){ .error = true };
        }

        aux[num_aux++] = string_c;
    }

    if(read == code_buffer_length){
        code_buffer_length = 0;

        while(true){
            c = get();

            if(c == 0){
                printf("error: Unterminated string\n");
                return (LexUnboundedResult){ .error = true };
            }

            if(c == '\n'){
                lex_line_number++;
            }

            if(escape){
                escape = false;

                if(c == 'n'){
                    c = '\n';
                } else if(c == '\t'){
                    c = '\t';
                }
            } else if(c == '"'){
                c = get();
                break;
            } else if(c == '\\'){
                escape = true;
                continue;
            }

            if(num_aux == AUX_CAPACITY){
                printf("Out of memory: Auxiliary memory used up\n");
                return (LexUnboundedResult){ .error = true };
            }
            aux[num_aux++] = c;
        }
    } else {
        read += 1;
        for(u32 i = read; i < code_buffer_length; i++){
            code_buffer[i - read] = code_buffer[i];
        }
        code_buffer_length -= read;
    }

    if(num_aux == AUX_CAPACITY){
        printf("Out of memory: Auxiliary memory used up\n");
        return (LexUnboundedResult){ .error = true };
    }

    aux[num_aux++] = '\0';
    return (LexUnboundedResult){ .error = false, .new_c = c };
}

