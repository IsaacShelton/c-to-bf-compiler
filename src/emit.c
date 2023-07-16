
#include <stdio.h>
#include "../include/storage.h"

void set_cell_to_value(char new_value, char old_value){
    if(new_value == old_value){
        // nothing to do
    } else if(new_value > old_value){
        // increase by difference
        printf("%d+", new_value - old_value);
    } else {
        // decrease by difference
        printf("%d-", old_value - new_value);
    }
}

void emit_print_aux_cstr(int string){
    printf("[-]"); // zero character cell
    
    char previous_value = 0;

    for(int i = string; aux[i]; i++){
        set_cell_to_value(aux[i], previous_value);
        printf(".");
        previous_value = aux[i];
    }
}

