
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
    case TOKEN_NOT:
        printf("!");
        break;
    case TOKEN_BIT_COMPLEMENT:
        printf("~");
        break;
    case TOKEN_INCREMENT:
        printf("++");
        break;
    case TOKEN_DECREMENT:
        printf("--");
        break;
    case TOKEN_TERNARY:
        printf("?");
        break;
    case TOKEN_COLON:
        printf(":");
        break;
    case TOKEN_IF:
        printf("if");
        break;
    case TOKEN_ELSE:
        printf("else");
        break;
    case TOKEN_WHILE:
        printf("while");
        break;
    case TOKEN_DO:
        printf("do");
        break;
    case TOKEN_TYPEDEF:
        printf("typedef");
        break;
    case TOKEN_STRUCT:
        printf("struct");
        break;
    case TOKEN_ADD_ASSIGN:
        printf("+=");
        break;
    case TOKEN_SUBTRACT_ASSIGN:
        printf("-=");
        break;
    case TOKEN_MULTIPLY_ASSIGN:
        printf("*=");
        break;
    case TOKEN_DIVIDE_ASSIGN:
        printf("/=");
        break;
    case TOKEN_MOD_ASSIGN:
        printf("%%=");
        break;
    case TOKEN_LSHIFT_ASSIGN:
        printf("<<=");
        break;
    case TOKEN_RSHIFT_ASSIGN:
        printf(">>=");
        break;
    case TOKEN_BIT_AND_ASSIGN:
        printf("&=");
        break;
    case TOKEN_BIT_OR_ASSIGN:
        printf("|=");
        break;
    case TOKEN_BIT_XOR_ASSIGN:
        printf("^=");
        break;
    case TOKEN_RETURN:
        printf("return");
        break;
    case TOKEN_BREAK:
        printf("break");
        break;
    default:
        printf("<unknown>");
    }
}

