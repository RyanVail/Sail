/*
 * This handles everything that involvs types.
 */

#ifndef TYPES_H
#define TYPES_H

#include<common.h>

#define IS_TYPE_INT(x) x.kind > 0x1 && x.kind < 0xc || x.kind == 1

typedef enum type_kind {
    VOID_TYPE,          // 0x0 // 0000
    BOOL_TYPE,          // 0x1 // 1000

    I8_TYPE,            // 0x2 // 1100
    U8_TYPE,            // 0x3 // 0010
    I16_TYPE,           // 0x4 // 1010
    U16_TYPE,           // 0x5 // 0110
    I32_TYPE,           // 0x6 // 1110
    U32_TYPE,           // 0x7 // 0001
    I64_TYPE,           // 0x8 // 1001
    U64_TYPE,           // 0x9 // 0101
    I128_TYPE,          // 0xa // 1101
    U128_TYPE,          // 0xb // 0011

    FLOAT_TYPE,         // 0xc // 0011
    DOUBLE_TYPE,        // 0xd // 1011
} type_kind;

/* struct type - This holds information about types
 * @ptr - The number of pointers
 * @kind - The type
 */
typedef struct type {
    u32 ptr;
    type_kind kind;
} type;

/*
 * This checks if type "_from" can be casted into type "_to" implicitily.
 */
bool type_can_implicitly_cast_to(type _from, type _to, bool error);

/*
 * This "fprint"s the type name.
 */
void print_type(type _type);

/*
 * This allows different front ends to set custom type names.
 */
void set_type_names(char*** _TYPE_NAMES);

/*
 * This resets the type names to the default
 */
void reset_type_names();

#endif
