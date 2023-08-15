
#include <stdio.h>
#include "../include/expression.h"

u0 expression_print_operator(ExpressionKind kind){
    switch(kind){
    case EXPRESSION_ASSIGN:
        printf("=");
        break;
    case EXPRESSION_ADD:
        printf("+");
        break;
    case EXPRESSION_SUBTRACT:
        printf("-");
        break;
    case EXPRESSION_MULTIPLY:
        printf("*");
        break;
    case EXPRESSION_DIVIDE:
        printf("/");
        break;
    case EXPRESSION_MOD:
        printf("%%");
        break;
    case EXPRESSION_LESS_THAN:
        printf("<");
        break;
    case EXPRESSION_GREATER_THAN:
        printf(">");
        break;
    case EXPRESSION_LSHIFT:
        printf("<<");
        break;
    case EXPRESSION_RSHIFT:
        printf(">>");
        break;
    default:
        printf("<unknown math operator>");
    }
}

u0 expression_print_operation_name(ExpressionKind kind){
    switch(kind){
    case EXPRESSION_ASSIGN:
        printf("assign");
        break;
    case EXPRESSION_ADD:
        printf("add");
        break;
    case EXPRESSION_SUBTRACT:
        printf("subtract");
        break;
    case EXPRESSION_MULTIPLY:
        printf("multiply");
        break;
    case EXPRESSION_DIVIDE:
        printf("divide");
        break;
    case EXPRESSION_MOD:
        printf("mod");
        break;
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
        printf("compare");
        break;
    case EXPRESSION_LSHIFT:
        printf("left shift");
        break;
    case EXPRESSION_RSHIFT:
        printf("right shift");
        break;
    default:
        printf("<unknown math operation>");
    }
}

ExpressionKind expression_kind_from_token_kind(TokenKind kind){
    switch(kind){
    case TOKEN_ASSIGN: return EXPRESSION_ASSIGN;
    case TOKEN_ADD: return EXPRESSION_ADD;
    case TOKEN_SUBTRACT: return EXPRESSION_SUBTRACT;
    case TOKEN_MULTIPLY: return EXPRESSION_MULTIPLY;
    case TOKEN_DIVIDE: return EXPRESSION_DIVIDE;
    case TOKEN_MOD: return EXPRESSION_MOD;
    case TOKEN_LESS_THAN: return EXPRESSION_LESS_THAN;
    case TOKEN_GREATER_THAN: return EXPRESSION_GREATER_THAN;
    case TOKEN_LSHIFT: return EXPRESSION_LSHIFT;
    case TOKEN_RSHIFT: return EXPRESSION_RSHIFT;
    default: return EXPRESSION_NONE;
    }
}

