/*
 * This handles everything that involves types.
 */

#include<types.h>
#include<common.h>
#include<intermediate/struct.h>

static char* DEFAULT_TYPE_NAMES[] = { "void", "bool", "i8", "u8", "i16", "u16",
"i32", "u32", "i64", "u64", "f32", "f64", "%", "%", "\0" };

static u32 DEFAULT_TYPE_SIZES[] = { 0, 1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 4 };

static char* DEFAULT_TYPE_MODIFIERS[] = { "unsigned", "signed", "const",
"volatile", "register", "static", "inline", "extern", "\0" };

static char** TYPE_MODIFIERS = DEFAULT_TYPE_MODIFIERS;
static char** TYPE_NAMES = DEFAULT_TYPE_NAMES;
static u32* TYPE_SIZES = DEFAULT_TYPE_SIZES;

// TODO: This should be in "intermediate" not here.
// TODO: The below function should change based on TYPE_SIZES rather than using
// hard coded values.
/*
 * This returns the lowest possible type a value can be.
 */
type_kind get_lowest_type(i64 value)
{
    if (value > 0) {
        if (value <= __UINT8_MAX__)
            return U8_TYPE;
        if (value <= __UINT16_MAX__)
            return U16_TYPE;
        if (value <= __UINT32_MAX__)
            return U32_TYPE;
        if (value <= __UINT64_MAX__)
            return U64_TYPE;
        return U32_TYPE;
    }

    if (value <= __INT8_MAX__ && value >= ~__INT8_MAX__)
        return I8_TYPE;
    if (value <= __INT16_MAX__ && value >= ~__INT16_MAX__)
        return I16_TYPE;
    if (value <= __INT32_MAX__ && value >= ~__INT32_MAX__)
        return I32_TYPE;
    if (value <= __INT64_MAX__ && value >= ~__INT64_MAX__)
        return I64_TYPE;
    return I32_TYPE;
}

/*
 * This takes in a type and a function that generates the contents of a struct
 * and returns the size of a type. If no "struct_generator" is provided this
 * not generate the size of a struct and will return "__UINT32_MAX__".
 * "struct_generator" is expected to generate both the padding of the struct and
 * put the full size of the struct into "byte_size" from a ptr to the struct.
 */
u32 get_size_of_type(type _type, void* struct_generator(intermediate_struct*))
{
    intermediate_struct* _struct;
    if (IS_TYPE_STRUCT(_type)) {
        /* Checking if this is a struct pointer. */
        if (_type.kind >> 16)
            return TYPE_SIZES[12];

        /* Generating the struct's size if it hasn't been inited yet. */
        _struct = find_struct(_type.ptr);
        if (_struct->byte_size == __UINT16_MAX__) {
            if (struct_generator == NULLPTR)
                return __UINT32_MAX__;
            struct_generator(_struct);
        }
        return _struct->byte_size;
    } else if (_type.ptr != 0) {
        /* If the type is a ptr this is the size of a ptr. */
        return TYPE_SIZES[12];
    } else {
        /* If the type is a normal type. */
        return TYPE_SIZES[_type.kind & 0xF];
    }
}

/*
 * This checks if type "_from" can be casted into type "_to" implicitily.
 * Returns true if "_from" can implicitly cast to "_to". Otherwise prints an
 * error.
 */
bool type_can_implicitly_cast_to(type _from, type _to)
{
    if (_from.kind == VOID_TYPE || _to.kind == VOID_TYPE)
        send_error("Usage of type `void` without cast");

    if (_to.ptr != _from.ptr)
        goto type_can_implicitly_cast_to_error_label;

    /* If only one type is a struct then send an error. */
    if (IS_TYPE_STRUCT(_to) ^ IS_TYPE_STRUCT(_from))
        goto type_can_implicitly_cast_to_error_label;
    else if ((IS_TYPE_STRUCT(_to) && IS_TYPE_STRUCT(_from))
    && (_to.kind >> 16 != _from.kind >> 16))
        goto type_can_implicitly_cast_to_error_label;

    /*
     * If these are ints and one is negative and the other is positive or the
     * "to" type is smaller than the "from" type there send an error.
     */
    if (IS_TYPE_INT(_to) && (IS_TYPE_INT(_from))
    && ((_to.kind < _from.kind) || ((_to.kind & 1) != (_from.kind & 1))))
            goto type_can_implicitly_cast_to_error_label;

    /* If only one type is a float / double. */
    if (IS_TYPE_FLOAT_OR_DOUBLE(_from) ^ IS_TYPE_FLOAT_OR_DOUBLE(_to))
        goto type_can_implicitly_cast_to_error_label;

    /* If "to" is a float and "from" is a double send an error. */
    if ((IS_TYPE_FLOAT_OR_DOUBLE(_from) && IS_TYPE_FLOAT_OR_DOUBLE(_to))
    && (_from.kind > _to.kind))
        goto type_can_implicitly_cast_to_error_label;

    return true;

    type_can_implicitly_cast_to_error_label:

    printf("\x1b[091mERROR:\x1b[0m Cannot implicity cast ");
    print_type_kind(_from, true);
    printf(": ");
    print_type(_from, true);
    printf(" to ");
    print_type_kind(_to, true);
    printf(": ");
    print_type(_to, true);
    printf("\n");
    exit(-1);
}

/*
 * This prints the type name.
 */
void print_type(type _type, bool graphical)
{
    /* Printing the inital coloring and '`'. */
    if (graphical)
        printf("\x1b[035m`");

    /* Getting the # of ptrs. */
    u32 ptr_count = IS_TYPE_STRUCT(_type) ? _type.kind >> 16 : _type.ptr;

    /* Printing the before ptrs (if present). */
    if (TYPE_NAMES[0xc][0] != '\0')
        for (u32 count = 0; count < ptr_count; count++)
            printf("%s", TYPE_NAMES[0xc]);

    /* Printing the type name. */
    if (IS_TYPE_STRUCT(_type))
        printf("%s", find_struct(_type.ptr)->name);
    else
        printf("%s", TYPE_NAMES[_type.kind]);

    /* Printing in the after ptrs (if present). */
    if (TYPE_NAMES[0xd][0] == '\0')
        return;
    for (u32 count = 0; count < ptr_count; count++)
        printf("%s", TYPE_NAMES[0xd]);

    /* Printing the ending color and '`'. */
    if (graphical)
        printf("`\x1b[0m");
}

/*
 * This prints the kind of type. "type" for native types, "struct" for structs,
 * and "enum" for enums.
 */
void print_type_kind(type _type, bool graphical)
{
    // TODO: These escape code colors should be macros in "errors.h".

    /* Printing the inital coloring and '`'. */
    if (graphical)
        printf("\x1b[090m");

    switch (_type.kind)
    {
    case STRUCT_TYPE:
        printf("struct");
        break;
    // case ENUM_TYPE:
        // printf("enum");
    default:
        if (_type.ptr)
            printf("pointer");
        else
            printf("type");
        break;
    }

    /* Printing the ending color and '`'. */
    if (graphical)
        printf("\x1b[0m");
}

/*
 * This scales the inputed value to the inputed type. This only works for values
 * no greater magnitude than the minimum value of the type to the power of two
 * minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type)
{
    i64 tmp_value = value >> (TYPE_SIZES[_type.kind] << 3);
    return (tmp_value == 0) ? (value) : (~tmp_value);
}

/*
 * This allows different front ends to set custom type names. The 0xe and 0xf
 * index of "TYPE_NAMES" should be the characters before and after variable
 * names to show pointers 0x0 means no character. The last string has to be
 * a string to a null pointers like so "\0".
 */
void set_type_names(char** _TYPE_NAMES)
{
    TYPE_NAMES = _TYPE_NAMES;
}

/*
 * This allows different front ends to set custom type modifier names. The first
 * modifier name is the unsigned identifier and the second is the unsigned
 * identifier. chars in place The rest of the type modifiers are the same as
 * those in the "type_kind" enum. The last char should be a '\0'.
 */
void set_type_modifier_names(char** _MODIFIER_NAMES)
{
    TYPE_MODIFIERS = _MODIFIER_NAMES;
}

/*
 * This allows different front ends / back ends to set custom type sizes. The
 * sizes should correspond with the "type_kind" enum rather than "TYPE_NAMES".
 * The last size is the size of pointers.
 */
void set_type_sizes(u32* _TYPE_SIZES)
{
    TYPE_SIZES = _TYPE_SIZES;
}

/*
 * This allows the front ends to get the type modifier's names.
 */
char** get_type_modifier_names()
{
    return TYPE_MODIFIERS;
}

/*
 * This allows the front ends to get the names of types.
 */
char** get_type_names()
{
    return TYPE_NAMES;
}

/*
 * This allows the front ends / back ends to get the sizes of types.
 */
u32* get_type_sizes()
{
    return TYPE_SIZES;
}

/*
 * This resets the type names to the default
 */
void reset_type_names()
{
    TYPE_NAMES = DEFAULT_TYPE_NAMES;
}
