
#include <stdbool.h>
#include <stdio.h>
#include "../include/token.h"
#include "../include/token_print.h"
#include "../include/lex.h"
#include "../include/io.h"
#include "../include/storage.h"

typedef struct {
    bool ok;
    Token token;
    int consumed;
} LexedToken;

typedef struct {
    char c;
    TokenKind kind;
} SimpleToken;

SimpleToken simple_tokens[7] = {
    (SimpleToken){ .c = '{', .kind = TOKEN_BEGIN },
    (SimpleToken){ .c = '}', .kind = TOKEN_END },
    (SimpleToken){ .c = '(', .kind = TOKEN_OPEN },
    (SimpleToken){ .c = ')', .kind = TOKEN_CLOSE },
    (SimpleToken){ .c = ';', .kind = TOKEN_SEMICOLON },
    (SimpleToken){ .c = '[', .kind = TOKEN_OPEN_BRACKET },
    (SimpleToken){ .c = ']', .kind = TOKEN_CLOSE_BRACKET },
};

static bool is_ident(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9');
}

int line_number = 1;

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
    int whitespace = 0;
    for(int i = 0; i < code_buffer_length; i++){
        char c = code_buffer[i];

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
    for(int i = 0; i < sizeof simple_tokens / sizeof(SimpleToken); i++){
        SimpleToken simple = simple_tokens[i];
        if(code_buffer[0] == simple.c){
            result.token.kind = simple.kind;
            result.consumed = 1;
            return result;
        }
    }

    char lead = code_buffer[0];

    // Handle strings
    if(lead == '"'){
        result.token.kind = TOKEN_STRING;
        result.consumed = 1;
        return result;
    }

    // Handle integers
    if(lead >= '0' && lead <= '9'){
        result.token.kind = TOKEN_INT;
        result.token.data = lead - '0';

        int i = 1;
        lead = code_buffer[i];
        while(lead >= '0' && lead <= '9'){
            lead = code_buffer[++i];
        }

        result.consumed = i;
        return result;
    }

    // Handle identifiers
    if(is_ident(lead)){
        result.token.kind = TOKEN_WORD;
        int i = 1;

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

    printf("error:%d: %c %d\n", line_number, code_buffer[0], code_buffer[0]);
    result.token.kind = TOKEN_ERROR;
    result.consumed = 1;
    return result;
}

int lex(){
    char c = get();

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
                token.line = line_number;
                tokens[num_tokens++] = token;
            } else {
                for(int i = 0; i < lexed.consumed; i++){
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

                for(int i = 0; i < lexed.consumed; i++){
                    aux[num_aux++] = code_buffer[i];
                }
                aux[num_aux++] = '\0';
            }

            for(int i = lexed.consumed; i < code_buffer_length; i++){
                code_buffer[i - lexed.consumed] = code_buffer[i];
            }
            code_buffer_length -= lexed.consumed;
        }

        // Special additional code for lexing strings, so they are not limited to code buffer capacity
        if(token.kind == TOKEN_STRING){
            tokens[num_tokens - 1].data = num_aux;

            int read = 0;

            for(; read < code_buffer_length; read++){
                char string_c = code_buffer[read];

                if(string_c == '"'){
                    break;
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

                    if(c == '"'){
                        break;
                    }

                    if(c == '\n'){
                        line_number++;
                    }

                    if(num_aux == AUX_CAPACITY){
                        printf("Out of memory: Auxiliary memory used up\n");
                        return 1;
                    }
                    aux[num_aux++] = c;
                }
            } else {
                read += 1;
                for(int i = read; i < code_buffer_length; i++){
                    if(code_buffer[i - read] == '\n'){
                        line_number++;
                    }
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

