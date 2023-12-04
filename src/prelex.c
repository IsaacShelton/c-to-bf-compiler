
#include <stdio.h>
#include "../include/utypes.h"
#include "../include/storage.h"

u0 print_lexed_construction(){
    u32 delta_aux = num_aux;
    u32 delta_tokens = num_tokens;

    printf("// Assume that num_aux + delta_aux < AUX_CAPACITY\n");
    for(u32 i = 0; i < num_aux; i++){
        printf("aux[(u32) %d] = %d;\n", i, aux[i]);
    }
    printf("num_aux += %d;\n", delta_aux);

    printf("// Assume that num_tokens + delta_tokens < AUX_CAPACITY\n");
    for(u32 i = 0; i < num_tokens; i++){
        u32 kind = (u32) (u8) tokens[i].kind;
        u32 data = tokens[i].data;
        printf("tokens[(u32) %d] = (Token){ .kind = (TokenKind) %d, .data = (u32) %u };\n", i, kind, (u32) data);
    }
    printf("num_tokens += (u32) %d;\n", delta_tokens);
}

