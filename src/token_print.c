
#include <stdio.h>
#include "../include/config.h"
#include "../include/token.h"
#include "../include/storage.h"

void token_print(Token token){
    switch(token.kind){
    case TOKEN_NONE:
        printf("<none>");
        break;
    case TOKEN_DONE:
        printf("<done>");
        break;
    case TOKEN_ERROR:
        printf("<error>");
        break;
    case TOKEN_WORD:
        printf("word: ");
        print_aux_cstr(token.data);
        break;
    case TOKEN_BEGIN:
        printf("{");
        break;
    case TOKEN_END:
        printf("}");
        break;
    case TOKEN_OPEN:
        printf("(");
        break;
    case TOKEN_CLOSE:
        printf(")");
        break;
    case TOKEN_STRING:
        printf("<string> \"");
        print_aux_cstr(token.data);
        printf("\"");
        break;
    case TOKEN_SEMICOLON:
        printf(";");
        break;
    case TOKEN_OPEN_BRACKET:
        printf("[");
        break;
    case TOKEN_CLOSE_BRACKET:
        printf("]");
        break;
    case TOKEN_INT:
        printf("%d", token.data);
        break;
    default:
        printf("<unknown>");
    }
}

