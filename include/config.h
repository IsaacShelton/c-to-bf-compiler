
#ifndef _PECK_CONFIG_H
#define _PECK_CONFIG_H

#define MEMORY_MULTIPLIER 1

#define MEGABYTES 1024 * 1024

// These values should be enough for regular programs
#define CODE_BUFFER_CAPACITY         MEMORY_MULTIPLIER * 256
#define TOKENS_CAPACITY              MEMORY_MULTIPLIER * 2*MEGABYTES / 8
#define AUX_CAPACITY                 MEMORY_MULTIPLIER * 2*MEGABYTES
#define TYPES_CAPACITY               MEMORY_MULTIPLIER * MEGABYTES / 10 / 8
#define UNIQUE_DIMENSIONS_CAPACITY   MEMORY_MULTIPLIER * 1024
#define FUNCTIONS_CAPACITY           MEMORY_MULTIPLIER * 2048
#define EXPRESSIONS_CAPACITY         MEMORY_MULTIPLIER * 4*MEGABYTES / 8
#define STATEMENTS_CAPACITY          MEMORY_MULTIPLIER * MEGABYTES / 4
#define OPERANDS_CAPACITY            MEMORY_MULTIPLIER * 1*MEGABYTES / 4
#define GLOBALS_CAPACITY             MEMORY_MULTIPLIER * 2048
#define TYPEDEFS_CAPACITY            MEMORY_MULTIPLIER * 2048
#define ARROWS_CAPACITY              MEMORY_MULTIPLIER * MEGABYTES / 4
#define TYPE_DEPENDENCIES_CAPACITY   MEMORY_MULTIPLIER * MEGABYTES / 8
#define ARITY_CAPACITY               32
#define CLOSES_NEEDED_CAPCAITY       MEMORY_MULTIPLIER * 4096
#define MAX_INITIALIZER_LIST_LENGTH  MEMORY_MULTIPLIER * 4096

#endif // _PECK_CONFIG_H
