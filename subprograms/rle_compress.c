
#include <stdio.h>

int main(int argc, const char **argv){
    unsigned int count = 1;
    char prev = '\0';
    char c = getc(stdin);

    while(c != EOF){
        if(c == prev){
            count++;
        } else if(prev){
            if(count > 1) printf("%d", count);
            putc(prev, stdout);
            count = 1;
        }
        prev = c;
        c = getc(stdin);
    }

    if(prev){
        if(count > 1) printf("%d", count);
        putc(prev, stdout);
    }

    return 0;
}

