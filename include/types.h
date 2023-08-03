/*
 * This handles everything that involves types.
 */

#ifndef TYPES_H
#define TYPES_H

#include<common.h>

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

/* This returns the first logical special type kind. */
#define TYPE_KIND_FIRST_SPECIAL (STRUCT_TYPE)

/* This returns true if the inputted type kind is special. */
#define IS_KIND_SPECIAL(x) ((x) > TYPE_KIND_FIRST_SPECIAL)

/* This returns true if the inputted type kind is special. */
#define IS_TYPE_SPECIAL(x) ((x).kind > TYPE_KIND_FIRST_SPECIAL)

/* This is index of the ptr size in type_sizes */
#define TYPE_PTR_INDEX (STRUCT_TYPE + 1)

/* This is the index of the first ptr char in type name arrays. */
#define TYPE_NAME_FIRST_PTR_INDEX (STRUCT_TYPE)

/* This is the index of the last ptr char in type name arrays. */
#define TYPE_NAME_LAST_PTR_INDEX (STRUCT_TYPE + 1)

/* The last logical internal type kind. */
#define TYPE_KIND_LAST_INTERNAL (STRUCT_TYPE)

/*
 * These are the kinds of types, use "type_kind" instead of this because it will
 * always be a u8. Any type kinds greater than that of a struct are considered
 * special / non native and specific to front ends.
 */
typedef enum _type_kind {
    VOID_TYPE,
    BOOL_TYPE,

    I8_TYPE,
    U8_TYPE,
    I16_TYPE,
    U16_TYPE,
    I32_TYPE,
    U32_TYPE,
    I64_TYPE,
    U64_TYPE,

    FLOAT_TYPE,
    DOUBLE_TYPE,
    FUNC_TYPE, // TODO: Everything has to support the function type now.
    STRUCT_TYPE,
    NO_TYPE = UINT8_MAX,
} _type_kind;

/* This ensure "type_kind" will always be a u8. */
typedef u8 type_kind;

// TODO: Extra data can be a union between "void*" and an u32 on 32 bit archs.
/* struct type - This represents a type
 * @extra_data: This is an extra data attached to this type this changes based
 * on the frontend and the type kind.
 * @ptr_count: This is the number of ptrs attached to this type.
 * @kind: This is the kind of this type.
 */
typedef struct type {
    u32 extra_data;
    u16 ptr_count;
    u8 flags;
    type_kind kind;
} type;

/* struct num - This can represent each integer value from (2^64-1) to -(2^64-1)
 * which encompasses the ranges of all supported integer types
 * @value: The magnitude of this number
 * @negative: If this number is negative
 */
typedef struct num {
    u64 magnitude;
    bool negative;
} num;

typedef struct intermediate_pass intermediate_pass;

/* This returns the lowest possible integer type the inputted value can be. */
type_kind get_lowest_type(i64 value);

/*
 * This returns the byte size of the inputted type either by using the
 * type_sizes array or for special types this will return the result of calling
 * the intermediate pass' type size handler function. If the type kind is
 * special or the intermediate pass is a NULLPTR or the intermediate pass' type
 * size handler function is a NULLPTR this will return UINT32_MAX.
 */
u32 type_get_size(intermediate_pass* _pass, type _type);

/*
 * This returns the numerical type of the inputted "_type" which is the type
 * used during operations. For example ptrs would return a type kind that has
 * the same behaviour as the ptr during numerical operations.
 */
type_kind get_operational_type(type _type);

/* This is the return from type_can_implicitly_cast_to. */
typedef enum type_cast_status {
    CAST_NO_ERROR,                  /* Types can be casted. */
    CAST_ERROR_VOID,                /* One of the types was a non ptr void. */
    CAST_ERROR_DIF_PTR,             /* The types have different ptr counts. */
    CAST_ERROR_PRECISION_LOST,      /* Precision can be lost by casting. */
    CAST_ERROR_STRUCT,              /* Only one of the types is a struct. */
    CAST_ERROR_DIF_STRUCT           /* Both Structs, with different data. */
} type_cast_status;

/*
 * This checks if type _from can be casted into type _to implicitly. Returns a
 * type_cast_status if _from can implicitly cast to _to. Otherwise returns
 * false. For special types this will call the inputted intermediate pass' type
 * conversion function handler if the inputted intermediate pass is not a
 * NULLPTR and its type conversion function is not a NULLPTR.
 */
type_cast_status type_can_implicitly_cast_to(type _from, type _to);

/*
 * This prints the type name. If the type is a special type this will call the
 * inputted intermediate pass' type printer function if the inputted
 * intermediate pass is not a NULLPTR and its type printer isn't either.
 */
void print_type(intermediate_pass* _pass, type _type);

/*
 * This prints the kind of type. If the type is a special type this will call
 * the inputted intermediate pass' type kind printer function if it the
 * intermediate pass is not a NULLPTR and it's type kind printer isn't either.
 */
void print_type_kind(intermediate_pass* _pass, type _type);

// TODO: The correctness of this comment needs to be verified
/*
 * This scales the inputted value to the inputted type. This only works for
 * values no greater magnitude than the maximum value of the type to the power
 * of two minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type);

#endif
