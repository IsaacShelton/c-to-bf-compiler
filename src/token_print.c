
#include <stdio.h>
#include "../include/config.h"
#include "../include/token.h"
#include "../include/storage.h"

u0 token_print(Token token, u1 safe){
    if(safe){
        printf("token: ");
    }

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
        printf("\"");
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
    case TOKEN_TRUE:
        printf("true");
        break;
    case TOKEN_FALSE:
        printf("false");
        break;
    case TOKEN_INT:
        printf("%d", token.data);
        break;
    case TOKEN_ASSIGN:
        printf("=");
        break;
    case TOKEN_ADD:
        printf("+");
        break;
    case TOKEN_SUBTRACT:
        printf("-");
        break;
    case TOKEN_MULTIPLY:
        printf("*");
        break;
    case TOKEN_DIVIDE:
        printf("/");
        break;
    case TOKEN_MOD:
        printf("%%");
        break;
    case TOKEN_EQUALS:
        printf("==");
        break;
    case TOKEN_NOT_EQUALS:
        printf("!=");
        break;
    case TOKEN_LESS_THAN:
        printf("<");
        break;
    case TOKEN_GREATER_THAN:
        printf(">");
        break;
    case TOKEN_LESS_THAN_OR_EQUAL:
        printf("<=");
        break;
    case TOKEN_GREATER_THAN_OR_EQUAL:
        printf(">=");
        break;
    case TOKEN_LSHIFT:
        printf("<<");
        break;
    case TOKEN_RSHIFT:
        printf(">>");
        break;
    case TOKEN_AND:
        printf("&&");
        break;
    case TOKEN_OR:
        printf("||");
        break;
    case TOKEN_BIT_AND:
        printf("&");
        break;
    case TOKEN_BIT_OR:
        printf("|");
        break;
    case TOKEN_BIT_XOR:
        printf("^");
        break;
    default:
        printf("<unknown>");
    }
}

