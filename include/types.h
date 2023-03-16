/*
 * This handles everything that involves types.
 */

#ifndef TYPES_H
#define TYPES_H

#include<common.h>
#include<intermediate/struct.h>

/* This returns true if the inputted type kind is an int. */
#define IS_KIND_INT(x) ((x & 0xF) >= BOOL_TYPE && (x & 0xF) <= U64_TYPE)

/* This returns true if the inputted type is an int. */
#define IS_TYPE_INT(x) (IS_KIND_INT((x).kind))

/* This returns true if the inputted type is negative. */
#define IS_TYPE_NEG(x) (IS_TYPE_INT(x) && !((x).kind & 1))

/* This retursn true if the inputted type kind is negative. */
#define IS_KIND_NEG(x) (IS_KIND_INT(x) && !(x & 1))

/* This returns true if the inputted type is a float or double type. */
#define IS_TYPE_FLOAT_OR_DOUBLE(x) \
    ((x).kind == FLOAT_TYPE || (x).kind == DOUBLE_TYPE)

/* This returns true if the inputted type kind is a float or double. */
#define IS_KIND_FLOAT_OR_DOUBLE(x) (x == FLOAT_TYPE || x == DOUBLE_TYPE)

/* This returns true if the inputted type kind is special. */
#define IS_TYPE_KIND_SPECIAL(x) ((x).kind > STRUCT_TYPE)

/* This returns true if the inputted type kind is special. */
#define IS_TYPE_SPECIAL(x) (x > STRUCT_TYPE)

/*
 * These are the kinds of types, use "type_kind" instead of this because it will
 * always be a u16. Any type kinds greater than that of a struct are considered
 * special / non native and specific to front ends.
 */
typedef enum _type_kind {
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

    FLOAT_TYPE,         // 0xa // 0101
    DOUBLE_TYPE,        // 0xb // 1101
    STRUCT_TYPE,        // 0xc // 1110
} _type_kind;

/* This is the byte size of each non special type. */
extern u8 type_sizes[STRUCT_TYPE];

/* This ensure "type_kind" will always be a u16. */
typedef u16 type_kind;

/* struct type - This represents a type
 * @extra_data: This is an extra data attached to this type, this is a hash
 * table key when compiling for 64 bit archs and a ptr when compiling for 32 bit
 * archs.
 * @ptr_count: This is the number of ptrs attached to this type.
 * @kind: This is the kind of this type.
 */
typedef struct type {
    #if PTRS_ARE_32BIT
    u32 extra_data;
    #else
    void* extra_data;
    #endif
    u16 ptr_count;
    type_kind kind;
} type;

/* This returns the lowest possible integer type the inputted value can be. */
type_kind get_lowest_type(i64 value);

/*
 * This returns the byte size of the inputted type either by using the
 * type_sizes array or for special types this will return the result of calling
 * the intermediate pass' type size handler function. If the type kind is
 * special or the intermediate pass is a NULLPTR or the intermediate pass' type
 * size handler function is a NULLPTR this will return __UINT32_MAX__.
 */
u32 type_get_size(intermediate_pass* _pass, type _type);

/*
 * This checks if type "_from" can be casted into type "_to" implicitly.
 * Returns true if "_from" can implicitly cast to "_to". Otherwise returns
 * false. For special types this will call the inputted intermediate pass'
 * type conversion function handler if the inputted intermediate pass is not a
 * NULLPTR and its type conversion function is not a NULLPTR.
 */
bool type_can_implicitly_cast_to(type _from, type _to);

/*
 * This prints the type name. If the type is a special type this will call the
 * inputted intermediate pass' type printer function if the inputted
 * intermediate pass is not a NULLPTR and its type printer isn't either.
 */
void print_type(intermediate_pass* _pass, type _type, bool graphical);

/*
 * This prints the kind of type. If the type is a special type this will call
 * the inputted intermediate pass' type kind printer function if it the
 * intermediate pass is not a NULLPTR and it's type kind printer isn't either.
 */
void print_type_kind(type _type, bool graphical);

// TODO: The correctness of this comment needs to be verified
/*
 * This scales the inputted value to the inputted type. This only works for
 * values no greater magnitude than the maximum value of the type to the power
 * of two minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type);

#endif
