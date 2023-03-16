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
/* This returns the lowest possible integer type the inputted value can be. */
type_kind get_lowest_type(i64 value)
{
    #if USE_PREDEF_TYPE_MAXES
    if (value > 0) {
        if (value <= __UINT8_MAX__)
            return U8_TYPE;
        if (value <= __UINT16_MAX__)
            return U16_TYPE;
        if (value <= __UINT32_MAX__)
            return U32_TYPE;
        return U64_TYPE;
    }
    if (value <= __INT8_MAX__ && value >= ~__INT8_MAX__)
        return I8_TYPE;
    if (value <= __INT16_MAX__ && value >= ~__INT16_MAX__)
        return I16_TYPE;
    if (value <= __INT32_MAX__ && value >= ~__INT32_MAX__)
        return I32_TYPE;
    return I64_TYPE;
    #else
    type_kind current_type;
    i64 current_max;
    if (value > 0) {
        for (current_type = U8_TYPE; current_type <= U64_TYPE; current_type+=2){
            /* This assumes that the byte width of an u64 is eight. */
            if (TYPE_SIZES[current_type] >= 8)
                return current_type;
            if (value <= ((u64)1 << ((u64)TYPE_SIZES[current_type]<<3))-1)
                return current_type;
        }
        return U64_TYPE;
    }
    for (current_type = I8_TYPE; current_type <= I64_TYPE; current_type+=2) {
        i64 current_max = ((u64)1 << ((u64)TYPE_SIZES[current_type])-1)-1;
        if (value <= current_max && value >= ~current_max)
            return current_type;
    }
    return I64_TYPE;
    #endif
}

/*
 * This returns the byte size of the inputted type either by using the
 * type_sizes array or for special types this will return the result of calling
 * the intermediate pass' type size handler function. If the type kind is
 * special or the intermediate pass is a NULLPTR or the intermediate pass' type
 * size handler function is a NULLPTR this will return __UINT32_MAX__.
 */
u32 type_get_size(intermediate_pass* _pass, type _type);
{
    // intermediate_struct* _struct;
    // if (IS_TYPE_STRUCT(_type)) {
    //     /* Checking if this is a struct pointer. */
    //     if (_type.kind >> 16)
    //         return TYPE_SIZES[12];

    //     /* Generating the struct's size if it hasn't been initted yet. */
    //     _struct = get_struct(_type.ptr);
    //     if (_struct->byte_size == __UINT16_MAX__) {
    //         if (struct_generator == NULLPTR)
    //             return __UINT32_MAX__;
    //         struct_generator(_struct);
    //     }
    //     return _struct->byte_size;
    if (_type.ptr_count != 0) {
        /* If the type is a ptr this is the size of a ptr. */
        return TYPE_SIZES[0];
    } else {
        /* If the type is a normal type. */
        return TYPE_SIZES[_type.kind & 0xF];
    }
    // TODO: Call intermediate function for this.
}

/* These are the errors which casting types. */
typedef enum type_cast_status {
    CAST_NO_ERROR,
    CAST_ERROR_VOID,
} type_cast_status;

// TODO: This should just return an error type
/*
 * This checks if type "_from" can be casted into type "_to" implicitly.
 * Returns a type_cast_status if "_from" can implicitly cast to "_to". Otherwise returns
 * false. For special types this will call the inputted intermediate pass'
 * type conversion function handler if the inputted intermediate pass is not a
 * NULLPTR and its type conversion function is not a NULLPTR.
 */
bool type_can_implicitly_cast_to(type _from, type _to)
{
    /* Voids without pointers cannot be used. */
    if ((_from.kind == VOID_TYPE && !_from.ptr_count) || \
    (_to.kind == VOID_TYPE && !_to.ptr_count))
        return false;

    /*
     * If these are normal types this makes sure they both have the same ptr
     * count. If these are structs this make sure they're both the same struct.
     */
    if (_to.ptr_count != _from.ptr_count)
        return false;

    /* If only one type is a struct then send an error. */
    if (IS_TYPE_STRUCT(_to) ^ IS_TYPE_STRUCT(_from))
        goto type_can_implicitly_cast_to_error_label;

    /* If the structs pointers don't match send an error. */
    if ((IS_TYPE_STRUCT(_to) && IS_TYPE_STRUCT(_from))
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

    return false;

    // TODO: This error should be reimplemented somewhere, and some how.
    // print_type_kind(_from, true);
    // printf(": ");
    // print_type(_pass, _from, true);
    // printf(" to ");
    // print_type_kind(_to, true);
    // printf(": ");
    // print_type(_pass, _to, true);
    // printf("\n");
    // exit(-1);
}

// TODO: This graphical input is dumb.
/*
 * This prints the type name. The inputted intermediate pass is used to get data
 * on custom types likes structs and enums, does nothing if it's a NULLPTR.
 */
void print_type(intermediate_pass* _pass, type _type, bool graphical)
{
    /* Printing the inital coloring and '`'. */
    if (graphical)
        printf("\x1b[035m`");

    /* Getting the # of ptrs. */
    u32 ptr_count = GET_TYPE_PTR_COUNT(_type);

    /* Printing the before ptrs (if present). */
    if (TYPE_NAMES[0xc][0] != '\0')
        for (u32 count = 0; count < ptr_count; count++)
            printf("%s", TYPE_NAMES[0xc]);

    /* Printing the type name. */
    if (IS_TYPE_STRUCT(_type))
        printf("%s", get_struct(_type.ptr)->name);
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
 * This scales the inputted value to the inputted type. This only works for
 * values no greater magnitude than the maximum value of the type to the power
 * of two minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type)
{
    i64 tmp_value = value >> (TYPE_SIZES[_type.kind] << 3);
    return (tmp_value == 0) ? (value) : (~tmp_value);
}

/*
 * This allows different frontends to set custom type names. The 0xe and 0xf
 * index of "TYPE_NAMES" should be the characters before and after variable
 * names to show pointers 0x0 means no character. The last string has to be
 * a string to a null pointers like so "\0".
 */
void set_type_names(char** _TYPE_NAMES)
{
    TYPE_NAMES = _TYPE_NAMES;
}

/*
 * This allows different frontends to set custom type modifier names. The first
 * modifier name is the unsigned identifier and the second is the unsigned
 * identifier. chars in place The rest of the type modifiers are the same as
 * those in the "type_kind" enum. The last char should be a '\0'.
 */
void set_type_modifier_names(char** _MODIFIER_NAMES)
{
    TYPE_MODIFIERS = _MODIFIER_NAMES;
}

/*
 * This allows different frontends / backends to set custom type sizes. The
 * sizes should correspond with the "type_kind" enum rather than "TYPE_NAMES".
 * The last size is the size of pointers.
 */
void set_type_sizes(u32* _TYPE_SIZES)
{
    #if USE_PREDEF_TYPE_MAXES && DEBUG
    send_error("Type sizes cannot be changed when \"USE_PREDEF_TYPE_MAXES\" is set to true.");
    #endif

    TYPE_SIZES = _TYPE_SIZES;
}

/*
 * This allows the frontends to get the type modifier's names.
 */
char** get_type_modifier_names()
{
    return TYPE_MODIFIERS;
}

/*
 * This allows the frontends to get the names of types.
 */
char** get_type_names()
{
    return TYPE_NAMES;
}

/*
 * This allows the frontends / backends to get the sizes of types.
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
