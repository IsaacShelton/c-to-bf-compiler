
#include <stdio.h>
#include "../include/io.h"
#include "../include/lex_line_comment.h"
#include "../include/storage.h"

LexUnboundedResult lex_line_comment(u8 c){
    u32 read = 0;

    for(; read < code_buffer_length; read++){
        u8 comment_c = code_buffer[read];

        if(comment_c == 0 || comment_c == '\n'){
            break;
        }
    }

    if(read == code_buffer_length){
        code_buffer_length = 0;

        do {
            c = get();
        } while(!(c == 0 || c == '\n'));
    } else {
        // Advance code buffer
        for(u32 i = read; i < code_buffer_length; i++){
            code_buffer[i - read] = code_buffer[i];
        }
        code_buffer_length -= read;
    }

    return (LexUnboundedResult){ .error = false, .new_c = c };
} 

