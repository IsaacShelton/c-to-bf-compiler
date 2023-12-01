
#ifndef _PECK_CONFIG_H
#define _PECK_CONFIG_H

#define MEMORY_MULTIPLIER 1

#define MEGABYTES 1024 * 1024

// These values should be enough for regular programs
#define CODE_BUFFER_CAPACITY         (u32)( MEMORY_MULTIPLIER * 256 )
#define TOKENS_CAPACITY              (u32)( MEMORY_MULTIPLIER * 2*MEGABYTES / 8 )
#define AUX_CAPACITY                 (u32)( MEMORY_MULTIPLIER * 2*MEGABYTES )
#define TYPES_CAPACITY               (u32)( MEMORY_MULTIPLIER * MEGABYTES / 10 / 8 )
#define UNIQUE_DIMENSIONS_CAPACITY   (u32)( MEMORY_MULTIPLIER * 1024 )
#define FUNCTIONS_CAPACITY           (u32)( MEMORY_MULTIPLIER * 2048 )
#define EXPRESSIONS_CAPACITY         (u32)( MEMORY_MULTIPLIER * 4*MEGABYTES / 8 )
#define STATEMENTS_CAPACITY          (u32)( MEMORY_MULTIPLIER * MEGABYTES / 4 )
#define OPERANDS_CAPACITY            (u32)( MEMORY_MULTIPLIER * 1*MEGABYTES / 4 )
#define GLOBALS_CAPACITY             (u32)( MEMORY_MULTIPLIER * 2048 )
#define TYPEDEFS_CAPACITY            (u32)( MEMORY_MULTIPLIER * 2048 )
#define TYPE_ALIASES_CAPACITY        (u32)( MEMORY_MULTIPLIER * 32 )
#define DEFINES_CAPACITY             (u32)( MEMORY_MULTIPLIER * 64 )
#define ARROWS_CAPACITY              (u32)( MEMORY_MULTIPLIER * MEGABYTES / 4 )
#define TYPE_DEPENDENCIES_CAPACITY   (u32)( MEMORY_MULTIPLIER * MEGABYTES / 8 )
#define ARITY_CAPACITY               32
#define CLOSES_NEEDED_CAPCAITY       (u32)( MEMORY_MULTIPLIER * 4096 )
#define MAX_INITIALIZER_LIST_LENGTH  (u32)( MEMORY_MULTIPLIER * 4096 )
#define DEFAULT_STACK_SIZE           (u32)( 256 )

#endif // _PECK_CONFIG_H
