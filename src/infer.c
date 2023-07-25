
#include "../include/infer.h"
#include "../include/storage.h"
#include "../include/function_infer.h"

u32 infer(){
    for(u32 i = 0; i < num_functions; i++){
        function_infer(functions[i]);
    }
    return 0;
}

