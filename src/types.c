/*
 * This handles everything that involvs types.
 */
#include<types.h>
#include<common.h>
#include<backend/intermediate/struct.h>

static char* DEFAULT_TYPE_NAMES[] = { "void", "bool", "i8", "u8", "i16", "u16",
"i32", "u32", "i64", "u64", "float", "double", "%", "%", "\0" };

static u32 DEFAULT_TYPE_SIZES[] = { 0, 1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8};

static char* DEFAULT_TYPE_MODIFIERS[] = { "unsigned", "signed", "const",
"volatile", "register", "static", "inline", "extern", "\0" };

static char** TYPE_MODIFIERS = DEFAULT_TYPE_MODIFIERS;
static char** TYPE_NAMES = DEFAULT_TYPE_NAMES;
static u32* TYPE_SIZES = DEFAULT_TYPE_SIZES;

// TODO: The below function should change based on TYPE_SIZES rather than using
// hard coded values.
/*
 * This returns the lowest possible type a value can be.
 */
type_kind get_lowest_type(i64 value)
{
    // TODO: Decide if this should stay in or not.
    // if (value == 0 || value == 1)
        // return BOOL_TYPE;

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

// TODO: The error input of the below function was only used for debug things I'm
// pretty sure and doesn't need to exist anymore.
/*
 * This checks if type "_from" can be casted into type "_to" implicitily.
 * Returns true if "_from" can implicitly cast to "_to".
 */
bool type_can_implicitly_cast_to(type _from, type _to, bool error)
{
    if (_from.kind == VOID_TYPE || _to.kind == VOID_TYPE) {
        if (!error)
            return false;
        send_error("Error usage of `void` type without cast");
    }

    if (_to.ptr != _from.ptr)
        goto type_can_implicitly_cast_to_error_label;

    if (IS_TYPE_INT(_to) && (IS_TYPE_INT(_from))
    && ((_to.kind < _from.kind) || ((_to.kind & 1) != (_from.kind & 1))))
            goto type_can_implicitly_cast_to_error_label;

    // TODO: Impliment floats and doubles.
    if (_from.kind == FLOAT_TYPE || _from.kind == DOUBLE_TYPE
    || _to.kind == FLOAT_TYPE || _to.kind == DOUBLE_TYPE)
        send_error("Floats & doubles aren't implimented yet.");

    return true;

    type_can_implicitly_cast_to_error_label:

    if (!error)
        return false;

    // TODO: All errors should be like this one!!
    printf("\x1b[091mERROR:\x1b[0m Cannot implicity cast ");
    print_type_kind(_from, true);
    printf(" ");
    print_type(_from, true);
    printf(" into ");
    print_type_kind(_to, true);
    printf(" ");
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
