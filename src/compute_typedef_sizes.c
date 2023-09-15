
#include <stdio.h>
#include <string.h>
#include "../include/compute_typedef_sizes.h"
#include "../include/storage.h"
#include "../include/type_print.h"
#include "../include/type_emit.h"

typedef struct {
    u32 from;
    u32 to;
} TypeDependency;

ErrorCode compute_typedef_sizes(){
    TypeDependency dependencies[TYPE_DEPENDENCIES_CAPACITY];
    u32 num_dependencies = 0;

    u32 outgoing[TYPEDEFS_CAPACITY];
    memset(outgoing, 0, sizeof outgoing);

    // Construct type dependencies
    for(u32 i = 0; i < num_typedefs; i++){
        TypeDef def = typedefs[i];

        for(u32 j = def.begin; j < def.begin + def.num_fields; j++){
            Expression expression = expressions[statements[j]];
            if(expression.kind != EXPRESSION_DECLARE) continue;

            Type type = types[operands[expression.ops]];

            if(!(
                aux_cstr_equals_u0(type.name)
             || aux_cstr_equals_u1(type.name)
             || aux_cstr_equals_u8(type.name)
             || aux_cstr_equals_u16(type.name)
             || aux_cstr_equals_u32(type.name)
            )){
                // Not trivial type
                
                u32 required = find_typedef(type.name);
                if(required >= TYPEDEFS_CAPACITY){
                    printf("\nerror on line %d: Type '", u24_unpack(expression.line));
                    type_print(type);
                    printf("' does not exist\n");
                    return 1;
                }

                if(num_dependencies == TYPE_DEPENDENCIES_CAPACITY){
                    printf("\nOut of memory: Exceeded maximum number of typedef dependencies\n");
                    return 1;
                }

                u1 already_exists = false;

                for(u32 k = 0; k < num_dependencies; k++){
                    if(dependencies[k].from == i && dependencies[k].to == required){
                        already_exists = true;
                        break;
                    }
                }

                if(!already_exists){
                    dependencies[num_dependencies++] = (TypeDependency){
                        .from = i,
                        .to = required,
                    };

                    outgoing[i]++;
                }
            }
        }
    }

    u1 progress = true;

    // Compute types that have no remaining dependencies
    while(progress){
        progress = false;
        
        // Compute sizes for all trivial typedefs
        for(u32 i = 0; i < num_typedefs; i++){
            if(!(outgoing[i] == 0 && typedefs[i].computed_size == -1)) continue;

            TypeDef def = typedefs[i];
            u32 size = 0;

            for(u32 j = 0; j < def.num_fields; j++){
                Expression expression = expressions[statements[def.begin + j]];
                if(expression.kind != EXPRESSION_DECLARE) continue;

                u32 field_size = type_sizeof_or_max(operands[expression.ops], expression.line);
                if(field_size == -1) return 1;

                size += field_size;
            }

            typedefs[i].computed_size = size;

            for(u32 j = 0; j < num_dependencies; j++){
                if(dependencies[j].to == i){
                    outgoing[dependencies[j].from]--;
                    progress = true;
                }
            }

            break;
        }
    }

    // Ensure all typedefs were processed
    for(u32 i = 0; i < num_typedefs; i++){
        if(outgoing[i] != 0){
            printf("\nerror on line %d: Type '", u24_unpack(typedefs[i].line));
            print_aux_cstr(typedefs[i].name);
            printf("' has infinite size\n");
            return 1;
        }
    }

    return 0;
}

