# Peck

A compiler which compiles a large subset of C to Brainf\*ck

Also it can compile itself in theory, however the compiled version is several petabytes uncompressed and hasn't been successfully run end-to-end yet.

We recommend you use our [blazing-fast-interpreter](https://github.com/IsaacShelton/blazing-fast-interpreter) in combination with this project, since other interpreters are simply not fast enough to handle some of the complicated programs that this compiler is capable of producing.

### Supported Features Include:
- recursion
- early returns
- early breaks
- early continues
- typedef structs
- typedef aliases
- typedef enums
- struct designated initializers
- fixed sized arrays
- array coalascing
- strings
- string literals
- array literals
- if and if-else
- while
- do-while
- for
- switch
- switch cases with fallthough
- ternaries
- sizeof for values
- sizeof for types
- break and continue
- functions (with support for parameters and return values of course)
- all regular math, logic, relational, and bitwise operators that C has
- type casts
- global variables (and optional initializers for them)
- single and multiline comments
- pre and post increment/decrement
- printf
- memcmp
- memcpy
- hex literals
- conditionless blocks
- type coalescing
- high quality error messages
- and more...

### Differences from regular C
- There are no function prototypes, you can call functions even if they're defined later in the file
- Pointers are not supported (although they could be added in the future)
- Function pointers are not supported
- Multiple files are not supported, you must `cat` them together and then pipe them into the compiler
- Unions are not supported
- Bitfields are not supported
- Array designated initializers are not supported
- All types must be typedef'd (no bare struct declarations are allowed)
- There are no signed types (although they could be added in the future)
- There are no `signed`/`unsigned` keywords
- There are no 64-bit types, only `u0`/`void`, `u8`/`char`, `u16`/`short`/`int`, and `u32`/`int`
- There are no dynamically sized arrays, all arrays have a compile-time known size.
- Strings are char arrays e.g. `u8[128]` or `char[128]`
- Arrays are allowed to grow in order to coalsce to bigger arrays of the same type (additional elements are zero initialized)
- Different standard library than `libc`
- Formats to `printf` must know be known at compile-time
