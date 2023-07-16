
#include <stdio.h>
#include <ctype.h>

int main(int argc, const char **argv){
    unsigned int count = 0;
    char c = getc(stdin);

    while(c != EOF){
        if(isdigit(c)){
            count = 10 * count + (c - '0');
        } else {
            putc(c, stdout);
            if(count > 0) while(--count) putc(c, stdout);
        }
        c = getc(stdin);
    }

    return 0;
}

