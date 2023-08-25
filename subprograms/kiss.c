
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_U24 16777215

static const char valid[8] = "><+-.,[]";
static const char LOWER_6_BITS = 0x3F;
static const int LOWER_12_BITS = 0xFFF;
static const char instruction_ascii = 34;
static const char quick_ascii = 42;
static const char numeric_ascii = 63;

char ahead[4096], behind[4096];
unsigned int ahead_i, behind_i, ahead_length;

static char get(){ char c = getc(stdin); return c == EOF ? 0 : c; }
static int min(int a, int b) { return a < b ? a : b; }

static bool is_bf_instruction(char c){
    for(int i = 0; i < sizeof valid; i++){
        if(c == valid[i]){
            return true;
        }
    }

    return false;
}

static bool is_kbf_instruction(char c){
    return c >= instruction_ascii && c < instruction_ascii + 8;
}

static void check_valid(char tag, char c){
    if(!is_kbf_instruction(c)){
        printf("%c: not valid %c\n", tag, c);
        exit(-1);
    }
}

static void write_behind(char c){
    check_valid('b', c);
    behind[behind_i] = c;
    behind_i = (behind_i + 1) & LOWER_12_BITS;
}

static void write_instruction(char c){
    for(int i = 0; i < sizeof valid; i++){
        if(c == valid[i]){
            char instruction = instruction_ascii + i;
            check_valid('w', instruction);
            putc(instruction, stdout);
            write_behind(instruction);
            return;
        }
    }
}

static void write_6bit(char c){
    putc(numeric_ascii + (c & LOWER_6_BITS), stdout);
}

static void write_quick_repetition(int number){
    putc(quick_ascii + number % 19, stdout);
    if(number >= 19){
        putc(quick_ascii + (number / 19) % 19, stdout);
    }
}

static void refill(){
    while(ahead_length < sizeof ahead){
        char c = get();

        if(is_bf_instruction(c)){
            ahead[ahead_i] = c;
            ahead_i = (ahead_i + 1) & LOWER_12_BITS;
            ahead_length++;
        } else if(c == 0){
            break;
        }
    }
}

typedef struct { int offset; int length; } Match;

static char to_normal_bf(char c){
    return (c - instruction_ascii) < 8 ? valid[(c - instruction_ascii) % 8] : -1;
}

static bool does_match_fit(int start, Match match){
    if(match.length == 0 || start + match.length > ahead_length){
        return false;
    }

    for(int i = 0; i < match.length; i++){
        char a = ahead[(ahead_i - ahead_length + start + i) & LOWER_12_BITS];
        char b = to_normal_bf(behind[(behind_i - match.offset + i) & LOWER_12_BITS]);

        if(a != b){
            return false;
        }
    }

    return true;
}

static Match find_match(){
    int best_offset = 0;
    int best_length = 0;

    for(int start = sizeof behind; start > 0; start--){
        int count = 0;

        while(count < sizeof behind){
            char a = ahead[(ahead_i - ahead_length + count) & LOWER_12_BITS];
            char b = to_normal_bf(behind[(behind_i - start + count) & LOWER_12_BITS]);

            if(a == b){
                count++;
            } else {
                break;
            }
        }

        if(count > best_length){
            best_offset = start;
            best_length = count;
        }
    }

    return (Match){ .offset = best_offset, .length = best_length };
}

static void write_repetition_encoding(Match match, int count){
    if(match.offset == 1 && match.length == 1 && count < 19*19){
        write_quick_repetition(count);

        for(int i = 0; i < min(count, 64); i++){
            write_behind(behind[(behind_i - 1) & LOWER_12_BITS]);
        }
    } else {
        write_6bit((match.offset >> 6) & LOWER_6_BITS);
        write_6bit(match.offset & LOWER_6_BITS);
        write_6bit(((match.length - 1) >> 6) & LOWER_6_BITS);
        write_6bit((match.length - 1) & LOWER_6_BITS);

        if(count < 19*19){
            write_quick_repetition(count);
        } else {
            write_6bit((count >> 18) & LOWER_6_BITS);
            write_6bit((count >> 12) & LOWER_6_BITS);
            write_6bit((count >> 6) & LOWER_6_BITS);
            write_6bit(count & LOWER_6_BITS);
        }
    }
}

static int get_match_score(Match match){
    int score = 0;

    while(does_match_fit(score, match)){
        score += match.length;
    }

    return score - 14;
}

static void encode_match(Match match){
    int count = 0;

    while(count < MAX_U24 - 1 && does_match_fit(0, match)){
        count++;
        ahead_length -= match.length;
        refill();
    }

    write_repetition_encoding(match, count);
}

int main(int argc, const char **argv){
    memset(ahead, 0, sizeof ahead);
    memset(behind, 0, sizeof behind);

    ahead_i = 0, behind_i = 0, ahead_length = 0;

    refill();

    while(ahead_length > 0){
        Match match = find_match();
        int match_score = get_match_score(match);
        int repeat_score = 12 + get_match_score((Match){ .offset = 1, .length = 1, });

        if(match_score > repeat_score && match_score > 0){
            encode_match(match);
        } else if(repeat_score > 0){
            char next = ahead[(ahead_i - ahead_length) & LOWER_12_BITS];
            write_instruction(next);
            ahead_length--;

            encode_match((Match){ .offset = 1, .length = 1 });
        } else {
            write_instruction(ahead[(ahead_i - ahead_length--) & LOWER_12_BITS]);
        }

        refill();
    }

    return 0;
}

