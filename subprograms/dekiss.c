
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char LOWER_6_BITS = 0x3F;
static const int LOWER_12_BITS = 0xFFF;

static char get(){ char c = getc(stdin); return c == EOF ? 0 : c; }
static char extract_instruction(char c){ return "><+-.,[]"[c - 34]; }
static char extract_6bit(char c){ return (c - 63) & LOWER_6_BITS; }
static char is_instruction(char c){ return c >= 34 && c <= 41; }
static char is_6bit_info(char c){ return c >= 63 && c <= 126; }
static char is_quick_repetition(char c){ return c >= 42 && c <= 60; }

static char behind[4096];
static unsigned int behind_i;

static void put_and_write_behind(char c){
    putc(c, stdout);
    behind[behind_i] = c;
    behind_i = (behind_i + 1) & LOWER_12_BITS;
}

static void perform_naked_quick_dupe(int count){
    char c = behind[(behind_i - 1) & LOWER_12_BITS];

    for(int i = 0; i < count; i++){
        put_and_write_behind(c);
    }
}

static void perform_full_repeat(int offset, int length, int count){
    for(int j = 0; j < count; j++){
        for(int i = 0; i < length; i++){
            putc(behind[(behind_i - offset + i) & LOWER_12_BITS], stdout);
        }
    }
}

typedef struct { char next; int number; } QuickNumber;

static QuickNumber parse_quick_number(char starting_c){
    char next = get();

    if(is_quick_repetition(next)){
        return (QuickNumber){
            .number = (int)(starting_c - 42) + 19 * (int)(next - 42),
            .next = get(),
        };
    } else {
        return (QuickNumber){
            .number = starting_c - 42,
            .next = next,
        };
    }
}

int main(int argc, const char **argv){
    behind_i = 0;
    memset(behind, 0, sizeof behind);

    char c = get();

    while(c){
        if(is_instruction(c)){
            put_and_write_behind(extract_instruction(c));
            c = get();
        } else if(is_6bit_info(c)){
            int offset = (extract_6bit(c) << 6) + extract_6bit(get());
            int length = (extract_6bit(get()) << 6) + extract_6bit(get()) + 1;

            c = get();
            int count = 0;

            if(is_quick_repetition(c)){
                QuickNumber quick = parse_quick_number(c);
                count = quick.number;
                c = quick.next;
            } else {
                count = extract_6bit(c) << 18;
                count += extract_6bit(get()) << 12;
                count += extract_6bit(get()) << 6;
                count += extract_6bit(get());
                c = get();
            }

            perform_full_repeat(offset, length, count);
        } else if(is_quick_repetition(c)){
            QuickNumber quick = parse_quick_number(c);
            perform_naked_quick_dupe(quick.number);
            c = quick.next;
        } else {
            c = get();
        }
    }

    return 0;
}

