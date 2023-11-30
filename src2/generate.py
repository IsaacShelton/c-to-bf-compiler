
import sys

# USAGE:
# /bin/cat include/*.h src/*.c | python3 src2/generate.py > src2/compiler.peck

lines = []

for line in sys.stdin:
    if line.startswith("#if") or line.startswith("#end") or line.startswith("#define _PECK") or line.startswith("extern") or line.startswith("#include"):
        continue

    line = (line.replace("fprintf(stderr, \"", "printf(\"\\n")
            .removeprefix("static ")
            .removeprefix("inline ")
            .replace("(void)", "()")
            .replace("(u8*)", "")
            .replace("&dimensions", "dimensions")
            .replace("typedef void u0;", "")
            .replace("typedef bool u1;", "")
            .replace("typedef uint8_t u8;", "")
            .replace("typedef uint16_t u16;", "")
            .replace("typedef uint32_t u32;", "")
            .replace("putchar", "put")
            )
    if line.startswith("typedef ") and not line.startswith("typedef enum ") and not line.startswith("typedef struct "):
        lines.insert(0, line)
    else:
        lines.append(line)

content = ''.join(lines).replace("""
u0 put(u8 c){
    put(c);
}

u8 get(){
    char c = getc(stdin);
    return c != EOF ? c : 0;
}
""", "")

print(content, end='')
