
#include <stdbool.h>
#include <stdio.h>
#include "../include/token.h"
#include "../include/token_print.h"
#include "../include/lex.h"
#include "../include/io.h"
#include "../include/storage.h"

typedef struct {
    u1 ok;
    Token token;
    u32 consumed;
} LexedToken;

typedef struct {
    u8 c;
    TokenKind kind;
} SimpleToken;

SimpleToken simple_tokens[8] = {
    (SimpleToken){ .c = '{', .kind = TOKEN_BEGIN },
    (SimpleToken){ .c = '}', .kind = TOKEN_END },
    (SimpleToken){ .c = '(', .kind = TOKEN_OPEN },
    (SimpleToken){ .c = ')', .kind = TOKEN_CLOSE },
    (SimpleToken){ .c = ';', .kind = TOKEN_SEMICOLON },
    (SimpleToken){ .c = '[', .kind = TOKEN_OPEN_BRACKET },
    (SimpleToken){ .c = ']', .kind = TOKEN_CLOSE_BRACKET },
    (SimpleToken){ .c = ',', .kind = TOKEN_NEXT },
};

static u1 is_ident(u8 c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9');
}

u32 line_number = 1;

LexedToken lex_main(){
    LexedToken result = (LexedToken){
        .ok = false,
        .token = (Token){
            .kind = TOKEN_NONE,
            .data = 0,
        },
        .consumed = 0,
    };

    // Handle empty
    if(code_buffer_length == 0){
        result.token.kind = TOKEN_DONE;
        return result;
    }

    // Handle whitespace
    u32 whitespace = 0;
    for(u32 i = 0; i < code_buffer_length; i++){
        u8 c = code_buffer[i];

        if(c == ' ' || c == '\n' || c == '\t'){
            whitespace++;
        } else {
            break;
        }
    }

    if(whitespace){
        result.consumed = whitespace;
        return result;
    }

    // Handle simple tokens
    for(u32 i = 0; i < sizeof simple_tokens / sizeof(SimpleToken); i++){
        SimpleToken simple = simple_tokens[i];

        if(code_buffer[0] == simple.c){
            result.token.kind = simple.kind;
            result.consumed = 1;
            return result;
        }
    }

    u8 lead = code_buffer[0];

    // Handle strings
    if(lead == '"'){
        result.token.kind = TOKEN_STRING;
        result.consumed = 1;
        return result;
    }

    // Handle integers
    if(lead >= '0' && lead <= '9'){
        u32 value = lead - '0';

        u32 i = 1;
        lead = code_buffer[i];
        while(lead >= '0' && lead <= '9'){
            value = 10 * value + lead - '0';
            lead = code_buffer[++i];
        }

        result.token.kind = TOKEN_INT;
        result.token.data = value;
        result.consumed = i;
        return result;
    }

    // Handle identifiers
    if(is_ident(lead)){
        result.token.kind = TOKEN_WORD;
        u32 i = 1;

        while(i < code_buffer_length){
            if(is_ident(code_buffer[i])){
                i++;
            } else {
                break;
            }
        }

        // NOTE: Anything words longer than code buffer size will be split in two
        result.consumed = i;
        return result;
    }

    // Handle equals
    if(lead == '='){
        if(code_buffer_length > 1 && code_buffer[1] == '='){
            result.token.kind = TOKEN_EQUALS;
            result.consumed = 2;
        } else {
            result.token.kind = TOKEN_ASSIGN;
            result.consumed = 1;
        }
        return result;
    }

    // Handle add
    if(lead == '+'){
        result.token.kind = TOKEN_ADD;
        result.consumed = 1;
        return result;
    }

    // Handle subtract
    if(lead == '-'){
        result.token.kind = TOKEN_SUBTRACT;
        result.consumed = 1;
        return result;
    }

    // Handle divide
    if(lead == '/'){
        if(code_buffer_length > 1 && code_buffer[1] == '/'){
            result.token.kind = TOKEN_COMMENT;
            result.consumed = 2;
            return result;
        }
    }

    printf("error on line %d: Unknown character `%c` (ASCII %d)\n", line_number, code_buffer[0], code_buffer[0]);
    result.token.kind = TOKEN_ERROR;
    result.consumed = 1;
    return result;
}

u32 lex(){
    u8 c = get();

    // Lex
    while(true){
        // (Re)-fill buffer
        while(code_buffer_length != CODE_BUFFER_CAPACITY && c){
            code_buffer[code_buffer_length++] = c;
            c = get();
        }

        // Process one token
        LexedToken lexed = lex_main();
        Token token = lexed.token;

        if(token.kind != TOKEN_DONE){
            if(token.kind != TOKEN_NONE){
                if(num_tokens == TOKENS_CAPACITY){
                    printf("Out of memory: Too many tokens\n");
                    return 1;
                }
                token.line = u24_pack(line_number);
                tokens[num_tokens++] = token;
            } else {
                for(u32 i = 0; i < lexed.consumed; i++){
                    if(code_buffer[i] == '\n') line_number++;
                }
            }

            // Remember word by injecting aux
            if(token.kind == TOKEN_WORD){
                tokens[num_tokens - 1].data = num_aux;

                if(num_aux + lexed.consumed + 1 >= AUX_CAPACITY){
                    printf("Out of memory: Auxiliary memory used up\n");
                    return 1;
                }

                for(u32 i = 0; i < lexed.consumed; i++){
                    aux[num_aux++] = code_buffer[i];
                }
                aux[num_aux++] = '\0';
            } else if(token.kind == TOKEN_ERROR){
                return 1;
            }

            for(u32 i = lexed.consumed; i < code_buffer_length; i++){
                code_buffer[i - lexed.consumed] = code_buffer[i];
            }
            code_buffer_length -= lexed.consumed;
        }

        // Special additional code for line comments
        if(token.kind == TOKEN_COMMENT){
            printf("error: Comments not implemented yet\n");
            return 1;
        }

        // Special additional code for lexing strings, so they are not limited to code buffer capacity
        // TODO: Refactor to merge buffer-only and past-buffer cases
        if(token.kind == TOKEN_STRING){
            tokens[num_tokens - 1].data = num_aux;

            u32 read = 0;
            u1 escape = false;

            for(; read < code_buffer_length; read++){
                u8 string_c = code_buffer[read];

                if(string_c == '\n'){
                    line_number++;
                }

                if(escape){
                    escape = false;

                    if(string_c == 'n'){
                        string_c = '\n';
                    } else if(string_c == '\t'){
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
                    return 1;
                }

                aux[num_aux++] = string_c;
            }

            if(read == code_buffer_length){
                code_buffer_length = 0;

                while(true){
                    c = get();

                    if(c == 0){
                        printf("error: Unterminated string\n");
                        return 1;
                    }

                    if(c == '\n'){
                        line_number++;
                    }

                    if(escape){
                        escape = false;

                        if(c == 'n'){
                            c = '\n';
                        } else if(c == '\t'){
                            c = '\t';
                        }
                    } else if(c == '"'){
                        break;
                    } else if(c == '\\'){
                        escape = true;
                        continue;
                    }

                    if(num_aux == AUX_CAPACITY){
                        printf("Out of memory: Auxiliary memory used up\n");
                        return 1;
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
                return 1;
            }
            aux[num_aux++] = '\0';
        }

        if(c == 0 && token.kind == TOKEN_DONE){
            break;
        }
    }

    return 0;
} 

