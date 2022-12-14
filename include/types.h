/*
 * This handles everything that involvs types.
 */

#ifndef TYPES_H
#define TYPES_H

#include<common.h>

#define IS_TYPE_INT(x) (x.kind >= BOOL_TYPE && x.kind <= U64_TYPE && !(x.ptr))
#define IS_KIND_INT(x) (x >= BOOL_TYPE && x <= U64_TYPE)
#define IS_TYPE_NEG(x) (!(x.ptr) && IS_TYPE_INT(x) && !(x.kind & 1))

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

    FLOAT_TYPE,         // 0xa // 0110
    DOUBLE_TYPE,        // 0xb // 0111
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
 * This returns the lowest possible type a value can be.
 */
type_kind get_lowest_type(i64 value);

/*
 * This checks if type "_from" can be casted into type "_to" implicitily.
 */
bool type_can_implicitly_cast_to(type _from, type _to, bool error);

/*
 * This "fprint"s the type name.
 */
void print_type(type _type);

/*
 * This allows different front ends to set custom type names. The 0xe and 0xf
 * index of "TYPE_NAMES" should be the characters before and after variable
 * names to show pointers 0x0 mean no character.
 */
void set_type_names(char** _TYPE_NAMES);

/*
 * This allows different front ends / back ends to set custom type sizes. The
 * sizes should correspond with the "type_kind" enum rather than "TYPE_NAMES".
 */
void set_type_sizes(u32* _TYPE_SIZES);

/*
 * This allows the front ends to get the names of types.
 */
char** get_type_names();

/*
 * This allows the front ends / back ends to get the sizes of types.
 */
u32* get_type_sizes();

/*
 * This resets the type names to the default
 */
void reset_type_names();

#endif
