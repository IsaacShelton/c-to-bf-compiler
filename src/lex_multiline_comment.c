
#include <stdio.h>
#include "../include/io.h"
#include "../include/lex_context.h"
#include "../include/lex_line_comment.h"
#include "../include/storage.h"

LexUnboundedResult lex_multiline_comment(u8 c){
    u32 read = 0;
    u8 prev = '\0';

    for(; read < code_buffer_length; read++){
        u8 comment_c = code_buffer[read];

        if(comment_c == 0){
            break;
        }

        if(comment_c == '\n'){
            lex_line_number++;
        }

        if(prev == '*' && comment_c == '/'){
            read++;
            break;
        }

        prev = comment_c;
    }

    if(read == code_buffer_length){
        code_buffer_length = 0;
        c = code_buffer[code_buffer_length - 1];

        do {
            if(c == '\n'){
                lex_line_number++;
            }

            prev = c;
            c = get();
        } while(!(c == 0 || (prev == '*' && c == '/')));

        if(c == '/'){
            c = get();
        }
    } else {
        // Advance code buffer
        for(u32 i = read; i < code_buffer_length; i++){
            code_buffer[i - read] = code_buffer[i];
        }
        code_buffer_length -= read;
    }

    return (LexUnboundedResult){ .error = false, .new_c = c };
} 

