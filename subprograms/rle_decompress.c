
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, const char **argv){
    unsigned int count = 0;
    bool has_count = false;
    char c = getc(stdin);

    while(c != EOF){
        if(isdigit(c)){
            count = 10 * count + (c - '0');
            has_count = true;
        } else if(has_count){
            while(count){
                putc(c, stdout);
                count--;
            }
            has_count = false;
        } else {
            putc(c, stdout);
        }

        c = getc(stdin);
    }

    return 0;
}

