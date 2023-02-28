/*
 * This handles everything that involves types.
 */

#ifndef TYPES_H
#define TYPES_H

#include<common.h>

#define IS_TYPE_INT(x) (x.kind >= BOOL_TYPE && x.kind <= U64_TYPE)
#define IS_KIND_INT(x) (x >= BOOL_TYPE && x <= U64_TYPE)
#define IS_TYPE_NEG(x) (IS_TYPE_INT(x) && !(x.kind & 1))
#define IS_KIND_NEG(x) (IS_KIND_INT(x) && !(x & 1))
#define IS_TYPE_STRUCT(x) (!(x.kind << 16 >> 16 ^ STRUCT_TYPE))
#define IS_KIND_STRUCT(x) (!((x << 16 >> 16) ^ STRUCT_TYPE))

#define IS_TYPE_FLOAT_OR_DOUBLE(x) (x.kind == FLOAT_TYPE||x.kind == DOUBLE_TYPE)
#define IS_KIND_FLOAT_OR_DOUBLE(x) (x == FLOAT_TYPE || x == DOUBLE_TYPE)

#define GET_TYPE_PTR_COUNT(x) (IS_TYPE_STRUCT((x))) ? (x.kind >> 16) : ((x).ptr)

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

    FLOAT_TYPE,         // 0xa // 0101
    DOUBLE_TYPE,        // 0xb // 1101
    STRUCT_TYPE,        // 0xc // 0011

    // Modifiers
    // Variables only
    COSNT_MODIFIER = 16,
    VOLATILE_MODIFIER = 32,
    REGISTER_MODIFIER = 64,
    // Variables and functions
    STATIC_MODIFIER = 128,
    // Functions only
    INLINE_MODIFIER = 256,
    EXTERN_MODIFIER = 512,
    // If this is a "STRUCT_TYPE" the next half of the u32 is the ptr count.
    /* This attempts to force this into being a u32. */
    TYPE_KIND_MAX = 2147483647
} type_kind;

/* struct type - This holds information about types
 * @ptr - The number of pointers / hash of the struct if "STRUCT_TYPE"
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

typedef struct intermediate_struct intermediate_struct;

/*
 * This takes in a type and a function that generates the contents of a struct
 * and returns the size of a type. If no "struct_generator" is provided this
 * not generate the size of a struct and will return "__UINT32_MAX__".
 * "struct_generator" is expected to generate both the padding of the struct and
 * put the full size of the struct into "byte_size" from a ptr to the struct.
 */
u32 get_size_of_type(type _type, void* struct_generator(intermediate_struct*));

/*
 * This checks if type "_from" can be casted into type "_to" implicitly.
 * Returns true if "_from" can implicitly cast to "_to". Otherwise prints an
 * error.
 */
bool type_can_implicitly_cast_to(type _from, type _to);

/*
 * This prints the type name.
 */
void print_type(type _type, bool graphical);

/*
 * This prints the kind of type. "type" for native types, "struct" for structs,
 * and "enum" for enums.
 */
void print_type_kind(type _type, bool graphical);

// TODO: This correctness of this comment needs to be verified
/*
 * This scales the inputted value to the inputted type. This only works for
 * values no greater magnitude than the minimum value of the type to the power
 * of two minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type);

/*
 * This allows different frontends to set custom type names. The 0xe and 0xf
 * index of "TYPE_NAMES" should be the characters before and after variable
 * names to show pointers 0x0 mean no character.
 */
void set_type_names(char** _TYPE_NAMES);

/*
 * This allows different frontends to set custom type modifier names. The first
 * modifier name is the unsigned identifier and the second is the unsigned
 * identifier. The rest of the type modifiers are the same as those in the
 * "type_kind" enum.
 */
void set_type_modifier_names(char** _MODIFIER_NAMES);

/*
 * This allows different frontends / backends to set custom type sizes. The
 * sizes should correspond with the "type_kind" enum rather than "TYPE_NAMES".
 */
void set_type_sizes(u32* _TYPE_SIZES);

/*
 * This allows the frontends to get the type modifier's names.
 */
char** get_type_modifier_names();

/*
 * This allows the frontends to get the names of types.
 */
char** get_type_names();

/*
 * This allows the frontends / backends to get the sizes of types.
 */
u32* get_type_sizes();

/*
 * This resets the type names to the default
 */
void reset_type_names();

#endif
