
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, const char **argv){
    unsigned int count = 0;
    bool has_count = false;
    char c = getc(stdin);

    size_t total_size = 0;

    while(c != EOF){
        if(isdigit(c)){
            count = 10 * count + (c - '0');
            has_count = true;
        } else if(has_count){
            total_size += count;
            has_count = false;
        } else {
            total_size++;
        }

        c = getc(stdin);
    }

    const char *sizes[] = {"Bytes", "Kilobytes", "Megabytes", "Gigabytes", "Terabytes", "Petabytes", "Exabytes", "Zettabytes"};
    int num_sizes = sizeof sizes / sizeof sizes[0];
    int order = 0;

    while(total_size >= 1024 && order < num_sizes){
        order++;
        total_size /= 1024;
    }

    printf("%zu %s\n", total_size, sizes[order]);
    return 0;
}

