
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
            .replace("int main(void){", "u0 main(){")
            .replace("(void)", "()")
            .replace("(u8*)", "")
            .replace("&dimensions", "dimensions")
            .replace("typedef void u0;", "")
            .replace("typedef bool u1;", "")
            .replace("typedef uint8_t u8;", "")
            .replace("typedef uint16_t u16;", "")
            .replace("typedef uint32_t u32;", "")
            .replace("putchar", "put")
            .replace(">= TYPES_CAPACITY", "== TYPES_CAPACITY")
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
""", "").replace("""
u0 emit_u32(u32 value){
    // Little endian
    emit_u8(value & 0xFF);
    emit_u8((value >> 8) & 0xFF);
    emit_u8((value >> 16) & 0xFF);
    emit_u8(value >> 24);
}
""", """
u0 emit_u32(u32 value){
    // Little endian
    emit_u8(value._0);
    emit_u8(value._1);
    emit_u8(value._2);
    emit_u8(value._3);
}
""").replace("""
u24 u24_pack(u32 value){
    u24 result;
    result.bytes[0] = (value >> 16) & 0xFF;
    result.bytes[1] = (value >> 8) & 0xFF;
    result.bytes[2] = value & 0xFF;
    return result;
}
""", """
u24 u24_pack(u32 value){
    u24 result;
    result.bytes[0] = value._0;
    result.bytes[1] = value._1;
    result.bytes[2] = value._2;
    return result;
}
""").replace("""
u32 u24_unpack(u24 value){
    return ((u32) value.bytes[0] << 16)
         + ((u32) value.bytes[1] << 8)
         + ((u32) value.bytes[2]);
}
""", """
u32 u24_unpack(u24 value){
    u32 result;
    result._0 = value.bytes[0];
    result._1 = value.bytes[1];
    result._2 = value.bytes[2];
    return result;
}
""").replace("""
u0 emit_u16(u16 value){
    // Little endian
    emit_u8(value & 0xFF);
    emit_u8((value >> 8) & 0xFF);
}
""", """
u0 emit_u16(u16 value){
    // Little endian
    emit_u8(value._0);
    emit_u8(value._1);
}
""")
print(content, end='')
