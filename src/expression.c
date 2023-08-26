
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
    case EXPRESSION_EQUALS:
        printf("==");
        break;
    case EXPRESSION_NOT_EQUALS:
        printf("!=");
        break;
    case EXPRESSION_LESS_THAN:
        printf("<");
        break;
    case EXPRESSION_GREATER_THAN:
        printf(">");
        break;
    case EXPRESSION_LESS_THAN_OR_EQUAL:
        printf("<=");
        break;
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
        printf(">=");
        break;
    case EXPRESSION_LSHIFT:
        printf("<<");
        break;
    case EXPRESSION_RSHIFT:
        printf(">>");
        break;
    case EXPRESSION_AND:
        printf("&&");
        break;
    case EXPRESSION_OR:
        printf("||");
        break;
    case EXPRESSION_BIT_AND:
        printf("&");
        break;
    case EXPRESSION_BIT_OR:
        printf("|");
        break;
    case EXPRESSION_BIT_XOR:
        printf("^");
        break;
    case EXPRESSION_NEGATE:
        printf("-");
        break;
    case EXPRESSION_NOT:
        printf("!");
        break;
    case EXPRESSION_BIT_COMPLEMENT:
        printf("~");
        break;
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_POST_INCREMENT:
        printf("++");
        break;
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_DECREMENT:
        printf("--");
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
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
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
    case EXPRESSION_AND:
        printf("AND");
        break;
    case EXPRESSION_OR:
        printf("OR");
        break;
    case EXPRESSION_BIT_AND:
        printf("bitwise-and");
        break;
    case EXPRESSION_BIT_OR:
        printf("bitwise-or");
        break;
    case EXPRESSION_BIT_XOR:
        printf("bitwise-xor");
        break;
    case EXPRESSION_NEGATE:
        printf("negate");
        break;
    case EXPRESSION_NOT:
        printf("not");
        break;
    case EXPRESSION_BIT_COMPLEMENT:
        printf("bitwise-complement");
        break;
    case EXPRESSION_PRE_INCREMENT:
    case EXPRESSION_POST_INCREMENT:
        printf("increment");
        break;
    case EXPRESSION_PRE_DECREMENT:
    case EXPRESSION_POST_DECREMENT:
        printf("decrement");
        break;
    default:
        printf("<unknown math operation>");
    }
}

ExpressionKind expression_kind_unary_prefix_from_token_kind(TokenKind kind){
    switch(kind){
    case TOKEN_SUBTRACT:
        return EXPRESSION_NEGATE;
    case TOKEN_NOT:
        return EXPRESSION_NOT;
    case TOKEN_BIT_COMPLEMENT:
        return EXPRESSION_BIT_COMPLEMENT;
    case TOKEN_INCREMENT:
        return EXPRESSION_PRE_INCREMENT;
    case TOKEN_DECREMENT:
        return EXPRESSION_PRE_DECREMENT;
    default:
        return EXPRESSION_NONE;
    }
}

ExpressionKind expression_kind_unary_postfix_from_token_kind(TokenKind kind){
    switch(kind){
    case TOKEN_INCREMENT:
        return EXPRESSION_POST_INCREMENT;
    case TOKEN_DECREMENT:
        return EXPRESSION_POST_DECREMENT;
    default:
        return EXPRESSION_NONE;
    }
}

ExpressionKind expression_kind_binary_from_token_kind(TokenKind kind){
    switch(kind){
    case TOKEN_ASSIGN:
        return EXPRESSION_ASSIGN;
    case TOKEN_ADD:
        return EXPRESSION_ADD;
    case TOKEN_SUBTRACT:
        return EXPRESSION_SUBTRACT;
    case TOKEN_MULTIPLY:
        return EXPRESSION_MULTIPLY;
    case TOKEN_DIVIDE:
        return EXPRESSION_DIVIDE;
    case TOKEN_MOD:
        return EXPRESSION_MOD;
    case TOKEN_EQUALS:
        return EXPRESSION_EQUALS;
    case TOKEN_NOT_EQUALS:
        return EXPRESSION_NOT_EQUALS;
    case TOKEN_LESS_THAN:
        return EXPRESSION_LESS_THAN;
    case TOKEN_GREATER_THAN:
        return EXPRESSION_GREATER_THAN;
    case TOKEN_LESS_THAN_OR_EQUAL:
        return EXPRESSION_LESS_THAN_OR_EQUAL;
    case TOKEN_GREATER_THAN_OR_EQUAL:
        return EXPRESSION_GREATER_THAN_OR_EQUAL;
    case TOKEN_LSHIFT:
        return EXPRESSION_LSHIFT;
    case TOKEN_RSHIFT:
        return EXPRESSION_RSHIFT;
    case TOKEN_AND:
        return EXPRESSION_AND;
    case TOKEN_OR:
        return EXPRESSION_OR;
    case TOKEN_BIT_AND:
        return EXPRESSION_BIT_AND;
    case TOKEN_BIT_OR:
        return EXPRESSION_BIT_OR;
    case TOKEN_BIT_XOR:
        return EXPRESSION_BIT_XOR;
    default:
        return EXPRESSION_NONE;
    }
}

ExpressionKind expression_kind_binary_from_assignment_token_kind(TokenKind kind){
    switch(kind){
    case TOKEN_ADD_ASSIGN:
        return EXPRESSION_ADD;
    case TOKEN_SUBTRACT_ASSIGN:
        return EXPRESSION_SUBTRACT;
    case TOKEN_MULTIPLY_ASSIGN:
        return EXPRESSION_MULTIPLY;
    case TOKEN_DIVIDE_ASSIGN:
        return EXPRESSION_DIVIDE;
    case TOKEN_MOD_ASSIGN:
        return EXPRESSION_MOD;
    case TOKEN_LSHIFT_ASSIGN:
        return EXPRESSION_LSHIFT;
    case TOKEN_RSHIFT_ASSIGN:
        return EXPRESSION_RSHIFT;
    case TOKEN_BIT_AND_ASSIGN:
        return EXPRESSION_BIT_AND;
    case TOKEN_BIT_OR_ASSIGN:
        return EXPRESSION_BIT_OR;
    case TOKEN_BIT_XOR_ASSIGN:
        return EXPRESSION_BIT_XOR;
    default:
        return EXPRESSION_NONE;
    }
}

