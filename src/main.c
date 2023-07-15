
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/config.h"
#include "../include/token.h"
#include "../include/io.h"
#include "../include/storage.h"
#include "../include/lex.h"
#include "../include/parse.h"

#include "../include/token_print.h"

int main(void){
    if(lex()) return 1;

    /*
    for(int i = 0; i < num_tokens; i++){
        token_print(tokens[i]);
        printf("\n");
    }
    printf("\n");
    */

    if(parse()) return 1;

    // Write

    return 0;
}

