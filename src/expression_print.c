
#include <stdio.h>
#include "../include/expression_print.h"
#include "../include/expression.h"
#include "../include/storage.h"
#include "../include/type_print.h"

u0 expression_print(Expression expression){
    switch(expression.kind){
    case EXPRESSION_RETURN: {
            printf("return");

            u32 return_value_index = expression.ops;

            if(return_value_index < EXPRESSIONS_CAPACITY){
                printf(" ");
                expression_print(expressions[return_value_index]);
            }
        }
        break;
    case EXPRESSION_DECLARE:
        type_print(types[operands[expression.ops]]);
        printf(" ");
        print_aux_cstr(operands[expression.ops + 1]);
        break;
    case EXPRESSION_PRINT_LITERAL:
        printf("print(\"");
        print_aux_cstr_escaped(expression.ops);
        printf("\")");
        break;
    case EXPRESSION_PRINT_ARRAY:
        printf("print(\"");
        expression_print(expressions[expression.ops]);
        printf("\")");
        break;
    case EXPRESSION_CALL:
    case EXPRESSION_PRINTF:
    case EXPRESSION_MEMCMP:
    case EXPRESSION_MEMCPY: {
            u32 name = operands[expression.ops];
            u32 arity = operands[expression.ops + 1];
            print_aux_cstr(name);
            printf("(");
            for(u32 i = 0; i < arity; i++){
                expression_print(expressions[operands[expression.ops + 2 + i]]);
                if(i + 1 != arity){
                    printf(", ");
                }
            }
            printf(")");
        }
        break;
    case EXPRESSION_IMPLEMENT_PUT:
        printf("<implementation of put>");
        break;
    case EXPRESSION_IMPLEMENT_PRINTU1:
        printf("<implementation of printu1>");
        break;
    case EXPRESSION_IMPLEMENT_PRINTU8:
        printf("<implementation of printu8>");
        break;
    case EXPRESSION_IMPLEMENT_GET:
        printf("<implementation of get>");
        break;
    case EXPRESSION_INT:
        printf("%d", expression.ops);
        break;
    case EXPRESSION_U1:
        printf(expression.ops ? "true" : "false");
        break;
    case EXPRESSION_U8:
        printf("%du8", expression.ops);
        break;
    case EXPRESSION_U16:
        printf("%du16", expression.ops);
        break;
    case EXPRESSION_U24:
        printf("%du24", expression.ops);
        break;
    case EXPRESSION_U32:
        printf("%du32", expression.ops);
        break;
    case EXPRESSION_VARIABLE:
        print_aux_cstr(expression.ops);
        break;
    case EXPRESSION_CAST:
        printf("(");
        type_print(types[operands[expression.ops]]);
        printf(") ");
        expression_print(expressions[operands[expression.ops + 1]]);
        break;
    case EXPRESSION_ASSIGN:
    case EXPRESSION_ADD:
    case EXPRESSION_SUBTRACT:
    case EXPRESSION_MULTIPLY:
    case EXPRESSION_DIVIDE:
    case EXPRESSION_MOD:
    case EXPRESSION_EQUALS:
    case EXPRESSION_NOT_EQUALS:
    case EXPRESSION_LESS_THAN:
    case EXPRESSION_GREATER_THAN:
    case EXPRESSION_LESS_THAN_OR_EQUAL:
    case EXPRESSION_GREATER_THAN_OR_EQUAL:
    case EXPRESSION_AND:
    case EXPRESSION_OR:
    case EXPRESSION_BIT_AND:
    case EXPRESSION_BIT_OR:
    case EXPRESSION_BIT_XOR:
    case EXPRESSION_LSHIFT:
    case EXPRESSION_RSHIFT: {
            u32 a = operands[expression.ops];
            u32 b = operands[expression.ops + 1];

            printf("(");
            expression_print(expressions[a]);
            printf(" ");
            expression_print_operator(expression.kind);
            printf(" ");
            expression_print(expressions[b]);
            printf(")");
        }
        break;
    case EXPRESSION_INDEX: {
            u32 a = operands[expression.ops];
            u32 b = operands[expression.ops + 1];

            expression_print(expressions[a]);
            printf("[");
            expression_print(expressions[b]);
            printf("]");
        }
        break;
    case EXPRESSION_NEGATE:
    case EXPRESSION_NOT:
    case EXPRESSION_BIT_COMPLEMENT:
        expression_print_operator(expression.kind);
        printf("(");
        expression_print(expressions[expression.ops]);
        printf(")");
        break;
    case EXPRESSION_PRE_INCREMENT:
        printf("++");
        expression_print(expressions[expression.ops]);
        break;
    case EXPRESSION_PRE_DECREMENT:
        printf("--");
        expression_print(expressions[expression.ops]);
        break;
    case EXPRESSION_POST_INCREMENT:
    case EXPRESSION_NO_RESULT_INCREMENT:
        expression_print(expressions[expression.ops]);
        printf("++");
        break;
    case EXPRESSION_POST_DECREMENT:
    case EXPRESSION_NO_RESULT_DECREMENT:
        expression_print(expressions[expression.ops]);
        printf("--");
        break;
    case EXPRESSION_TERNARY:
        printf("(");
        expression_print(expressions[operands[expression.ops]]);
        printf(" ? ");
        expression_print(expressions[operands[expression.ops + 1]]);
        printf(" : ");
        expression_print(expressions[operands[expression.ops + 2]]);
        printf(")");
        break;
    case EXPRESSION_MEMBER:
        expression_print(expressions[operands[expression.ops]]);
        printf(".");
        print_aux_cstr(operands[expression.ops + 1]);
        break;
    case EXPRESSION_STRING:
        printf("\"");
        print_aux_cstr_escaped(expression.ops);
        printf("\"");
        break;
    case EXPRESSION_BREAK:
        printf("break");
        break;
    case EXPRESSION_CONTINUE:
        printf("continue");
        break;
    case EXPRESSION_SIZEOF_TYPE:
    case EXPRESSION_SIZEOF_TYPE_U8:
    case EXPRESSION_SIZEOF_TYPE_U16:
    case EXPRESSION_SIZEOF_TYPE_U24:
    case EXPRESSION_SIZEOF_TYPE_U32:
        printf("sizeof");
        if(expression.kind == EXPRESSION_SIZEOF_TYPE_U8){
            printf("<u8>");
        } else if(expression.kind == EXPRESSION_SIZEOF_TYPE_U16){
            printf("<u16>");
        } else if(expression.kind == EXPRESSION_SIZEOF_TYPE_U24){
            printf("<u24>");
        } else if(expression.kind == EXPRESSION_SIZEOF_TYPE_U32){
            printf("<u32>");
        }
        printf("(");
        type_print(types[expression.ops]);
        printf(")");
        break;
    case EXPRESSION_SIZEOF_VALUE:
    case EXPRESSION_SIZEOF_VALUE_U8:
    case EXPRESSION_SIZEOF_VALUE_U16:
    case EXPRESSION_SIZEOF_VALUE_U24:
    case EXPRESSION_SIZEOF_VALUE_U32:
        printf("sizeof");
        if(expression.kind == EXPRESSION_SIZEOF_VALUE_U8){
            printf("<u8>");
        } else if(expression.kind == EXPRESSION_SIZEOF_VALUE_U16){
            printf("<u16>");
        } else if(expression.kind == EXPRESSION_SIZEOF_VALUE_U24){
            printf("<u24>");
        } else if(expression.kind == EXPRESSION_SIZEOF_VALUE_U32){
            printf("<u32>");
        }
        printf(" ");
        expression_print(expressions[expression.ops]);
        break;
    case EXPRESSION_CASE:
        if(operands[expression.ops] < EXPRESSIONS_CAPACITY){
            printf("case ");
            expression_print(expressions[operands[expression.ops]]);
        } else {
            printf("default");
        }
        break;
    case EXPRESSION_ARRAY_INITIALIZER: {
            u32 length = operands[expression.ops];
            printf("{");

            for(u32 i = 0; i < length; i++){
                if(i != 0){
                    printf(", ");
                }

                expression_print(expressions[operands[expression.ops + 1 + i]]);
            }

            printf("}");
        }
        break;
    case EXPRESSION_STRUCT_INITIALIZER: {
            u32 type = operands[expression.ops];
            u32 length = operands[expression.ops + 1];
            printf("(");
            type_print(types[type]);
            printf("){ ");

            for(u32 i = 0; i < length; i++){
                if(i != 0){
                    printf(", ");
                }

                expression_print(expressions[operands[expression.ops + 2 + i]]);
            }

            printf(" }");
        }
        break;
    case EXPRESSION_FIELD_INITIALIZER: {
            printf(".");
            print_aux_cstr(operands[expression.ops]);
            printf(" = ");
            expression_print(expressions[operands[expression.ops + 1]]);
        }
        break;
    case EXPRESSION_ENUM_VARIANT: {
            print_aux_cstr(expression.ops);
            printf(",");
        }
        break;
    case EXPRESSION_PANICLOOP: {
            printf("panicloop()");
       }
       break;
    default:
        printf("<unknown expression>");
    }
}

