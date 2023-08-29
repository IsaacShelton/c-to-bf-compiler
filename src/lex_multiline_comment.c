
#include <stdio.h>
#include "../include/io.h"
#include "../include/lex_context.h"
#include "../include/lex_line_comment.h"
#include "../include/storage.h"

LexUnboundedResult lex_multiline_comment(u8 c){
    u32 read = 0;
    u8 prev = '\0';
    u32 start_line = lex_line_number;
    u8 comment_c = '\0';

    for(; read < code_buffer_length; read++){
        comment_c = code_buffer[read];
        
        if(comment_c == 0){
            printf("\nerror on line %d: Multi-line comment never closed\n", start_line);
            return (LexUnboundedResult){ .error = true, .new_c = c };
        }

        if(comment_c == '\n'){
            lex_line_number++;
        }

        if(prev == '*' && comment_c == '/'){
            break;
        }

        prev = comment_c;
    }

    if(read < code_buffer_length){
        read++;

        // Advance code buffer
        for(u32 i = read; i < code_buffer_length; i++){
            code_buffer[i - read] = code_buffer[i];
        }
        code_buffer_length -= read;
    } else {
        // Not found in code buffer

        code_buffer_length = 0;
        c = get();

        while(!(c == 0 || (prev == '*' && c == '/'))){
            if(c == '\n'){
                lex_line_number++;
            }

            prev = c;
            c = get();
        }

        if(c == 0){
            printf("\nerror on line %d: Multi-line comment never closed\n", start_line);
            return (LexUnboundedResult){ .error = true, .new_c = c };
        }

        if(c == '/'){
            c = get();
        }
    }

    return (LexUnboundedResult){ .error = false, .new_c = c };
} 

