/*
 * This handles everything that involvs types.
 */

#include<types.h>
#include<common.h>

static char* DEFAULT_TYPE_NAMES[] = { "void", "bool", "u8", "i8", "u16", "i16", "u32",
"i32", "u64", "i64", "u128", "i128", "float", "double", "%", "%", "\0" };

// TODO: This doesn't need to be that many pointers.
static char** TYPE_NAMES = DEFAULT_TYPE_NAMES;

/*
 * This returns the lowest possible type a value can be.
 */
type_kind get_lowest_type(i128 value)
{
    if (value >= 0 && value <= 1)
        return BOOL_TYPE;
    if (value >= __INT8_MAX__ && value <= ~__INT8_MAX__)
        return I8_TYPE;
    if (value >= __INT16_MAX__ && value <= ~__INT16_MAX__)
        return I16_TYPE;
    if (value >= __INT32_MAX__ && value <= ~__INT32_MAX__)
        return I32_TYPE;
    if (value >= __INT64_MAX__ && value <= ~__INT64_MAX__)
        return I64_TYPE;
    if (value >= __LONG_LONG_MAX__ && value <= ~__LONG_LONG_MAX__)
        return I128_TYPE;
    return I32_TYPE;
}

/*
 * This checks if type "_from" can be casted into type "_to" implicitily.
 */
bool type_can_implicitly_cast_to(type _from, type _to, bool error)
{
    if (_from.kind == VOID_TYPE || _to.kind == VOID_TYPE) {
        if (!error)
            return false;
        send_error("Error usage of `void` type without cast");
    }

    if (_to.ptr != _from.ptr) {
        if (!error)
            return false;
        goto type_can_implicitly_cast_to_error_label;
    }

    if (IS_TYPE_INT(_to) && (IS_TYPE_INT(_from))) {
        if (_to.kind < _from.kind || _to.kind & 1 != _from.kind & 1) {
            if (!error)
                return false;
            goto type_can_implicitly_cast_to_error_label;
        }
    }

    if (_from.kind == FLOAT_TYPE || _from.kind == DOUBLE_TYPE
    || _to.kind == FLOAT_TYPE || _to.kind == DOUBLE_TYPE)
        send_error("Floats & doubles aren't implimented yet.");

    return true;

    type_can_implicitly_cast_to_error_label:
        printf("Error cannot implicity cast type `");
        print_type(_from);
        printf("` into type `");
        print_type(_to);
        printf("`.\n");
        exit(-1);
}

/*
 * This "fprint"s the type name.
 */
void print_type(type _type)
{
    if (TYPE_NAMES[0xe][0] != '\0')
        for (u32 count = 0; count < _type.ptr; count++)
            printf("%s", TYPE_NAMES[0xe]);

    printf("%s", TYPE_NAMES[_type.kind]);

    if (TYPE_NAMES[0xf][0] == '\0')
        return;

    for (u32 count = 0; count < _type.ptr; count++)
        printf("%s", TYPE_NAMES[0xf]);
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
 * This allows the front ends to get the names of types.
 */
char** get_type_names()
{
    return TYPE_NAMES;
}

/*
 * This resets the type names to the default
 */
void reset_type_names()
{
    TYPE_NAMES = DEFAULT_TYPE_NAMES;
}
