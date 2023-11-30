
import sys

# USAGE:
# /bin/cat include/*.h src/*.c | python3 src2/generate.py > src2/compiler.peck

for line in sys.stdin:
    if line.startswith("#if") or line.startswith("#end") or line.startswith("#define _PECK") or line.startswith("extern") or line.startswith("#include"):
        continue
    print(line.replace("fprintf(stderr, \"", "printf(\"\\n").removeprefix("static ").removeprefix("inline ").replace("(void)", "()").replace("(u8*)", "").replace("&dimensions", "dimensions"), end='')

