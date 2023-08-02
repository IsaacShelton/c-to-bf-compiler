
# Types

### Primitives
- `u0`
- `u1`
- `u8`
- `u16`
- `u32`

### Arrays
Array types are postfixed with their dimensions. Examples:

- `u8[256]`
- `u8[16][16]`
- `u8[8][8][4]`
- `u8[4][4][4][4]`

### User-defined types
User defined types are defined using the `typedef` keyword.

### Aliases
Alias types can be declared using C-like syntax. Unlike in C, type aliases are always strong and aren't implicitly converted.

```
typedef u8 Age;
```

Literals values will however have implicit conversion when applicable. Numeric type aliases will also work with numeric operations within
their own type. For example, the following is allowed:

```
typedef u8 Age;

// ...

Age age = 32;
age += 10;
```

### Structs
Struct types can be declared using C-like syntax. Unlike in C, typedef is required. Examples:

```
typedef struct {
    u8 first_name[64];
    u8 last_name[64];
    u8 age;
} Person;
```

