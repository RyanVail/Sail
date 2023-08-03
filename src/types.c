/*
 * This handles everything that involves types.
 */

#include<types.h>
#include<intermediate/struct.h>
#include<cli.h>
#include<archs.h>

/* This returns the lowest possible integer type the inputted value can be. */
type_kind get_lowest_type(i64 value)
{
    if (value > 0) {
        if (value <= UINT8_MAX)
            return U8_TYPE;
        if (value <= UINT16_MAX)
            return U16_TYPE;
        if (value <= UINT32_MAX)
            return U32_TYPE;
        return U64_TYPE;
    }
    if (value <= INT8_MAX && value >= ~INT8_MAX)
        return I8_TYPE;
    if (value <= INT16_MAX && value >= ~INT16_MAX)
        return I16_TYPE;
    if (value <= INT32_MAX && value >= ~INT32_MAX)
        return I32_TYPE;
    return I64_TYPE;
}

/*
 * This returns the byte size of the inputted type either by using the
 * type_sizes array or for special types this will return the result of calling
 * the intermediate pass' type size handler function. If the type kind is
 * special or the intermediate pass is a NULLPTR or the intermediate pass' type
 * size handler function is a NULLPTR this will return UINT32_MAX.
 */
u32 type_get_size(intermediate_pass* _pass, type _type)
{
    // TODO: Reimplement this or remove it
    // intermediate_struct* _struct;
    // if (IS_TYPE_STRUCT(_type)) {
    //     /* Checking if this is a struct pointer. */
    //     if (_type.kind >> 16)
    //         return TYPE_SIZES[12];

    //     /* Generating the struct's size if it hasn't been initted yet. */
    //     _struct = get_struct(_type.ptr);
    //     if (_struct->byte_size == UINT16_MAX) {
    //         if (struct_generator == NULLPTR)
    //             return UINT32_MAX;
    //         struct_generator(_struct);
    //     }
    //     return _struct->byte_size;

    /* If this type is special the intermediate pass should handle it. */
    if (IS_TYPE_SPECIAL(_type)) {
        /* Making sure this intermediate pass is valid. */
        if (_pass == NULLPTR
        || _pass->data.front_end->type_size_func == NULLPTR)
            return UINT32_MAX;
        return (*_pass->data.front_end->type_size_func)(_pass, _type);
    }

    /* Returning the size of a ptr if this is a ptr. */
    if (_type.ptr_count)
        return arch_ptr_equivalents[global_cli_options.target];

    switch (_type.kind)
    {
    case VOID_TYPE:
        return 0;
    case BOOL_TYPE:
    case I8_TYPE:
    case U8_TYPE:
        return 1;
    case I16_TYPE:
    case U16_TYPE:
        return 2;
    case I32_TYPE:
    case U32_TYPE:
    case FLOAT_TYPE:
        return 4;
    case I64_TYPE:
    case U64_TYPE:
    case DOUBLE_TYPE:
        return 8;
    default:
        __builtin_unreachable();
    }
}

/*
 * This returns the numerical type of the inputted "_type" which is the type
 * used during operations. For example ptrs would return a type kind that has
 * the same behaviour as the ptr during numerical operations.
 */
type_kind get_operational_type(type _type)
{
    if (_type.ptr_count)
        return arch_ptr_equivalents[global_cli_options.target];

    if (IS_TYPE_FLOAT_OR_DOUBLE(_type) || IS_TYPE_INT(_type))
        return _type.kind;

    return NO_TYPE;
}

/*
 * This checks if type _from can be casted into type _to implicitly. Returns a
 * type_cast_status if _from can implicitly cast to _to. Otherwise returns
 * false. For special types this will call the inputted intermediate pass' type
 * conversion function handler if the inputted intermediate pass is not a
 * NULLPTR and its type conversion function is not a NULLPTR.
 */
type_cast_status type_can_implicitly_cast_to(type _from, type _to)
{
    /* Voids without pointers cannot be used. */
    if ((_from.kind == VOID_TYPE && !_from.ptr_count) || \
    (_to.kind == VOID_TYPE && !_to.ptr_count))
        return CAST_ERROR_VOID;

    /* Making sure the ptr count is the same. */
    if (_to.ptr_count != _from.ptr_count)
        return CAST_ERROR_DIF_PTR;

    /* If only one type is a struct they can't be casted. */
    if (_to.kind == STRUCT_TYPE ^ _from.kind == STRUCT_TYPE)
        return CAST_ERROR_STRUCT;

    /* If the structs pointers don't match send an error. */
    if ((_to.kind == STRUCT_TYPE && _from.kind == STRUCT_TYPE)
    && (_to.extra_data != _from.extra_data))
        return CAST_ERROR_DIF_STRUCT;

    /*
     * If these are ints and one is negative and the other is positive or the
     * to type is smaller than the from type they can't be casted implicitly.
     */
    if (IS_TYPE_INT(_to) && (IS_TYPE_INT(_from))
    && ((_to.kind < _from.kind) || ((_to.kind & 1) != (_from.kind & 1))))
        return CAST_ERROR_PRECISION_LOST;

    /*
     * Floats and doubles cannot operate on non floating point types without
     * explicit casting.
     */
    if (IS_TYPE_FLOAT_OR_DOUBLE(_from) ^ IS_TYPE_FLOAT_OR_DOUBLE(_to))
        return CAST_ERROR_PRECISION_LOST;

    /*
     * If to is a float and from is a double precision can be lost so they
     * cannot be implicitly casted.
     */
    if ((IS_TYPE_FLOAT_OR_DOUBLE(_from) && IS_TYPE_FLOAT_OR_DOUBLE(_to))
    && (_from.kind > _to.kind))
        return CAST_ERROR_PRECISION_LOST;

    return CAST_NO_ERROR;

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

/*
 * This prints the type name. If the type is a special type this will call the
 * inputted intermediate pass' type printer function if the inputted
 * intermediate pass is not a NULLPTR and its type printer isn't either.
 */
void print_type(intermediate_pass* _pass, type _type)
{
    /* Printing the inital coloring and '`'. */
    printf("\x1b[035m`");

    char** type_names = _pass->data.front_end->type_names;

    /* Printing the before ptrs (if present). */
    if (type_names[TYPE_NAME_FIRST_PTR_INDEX][0] != '\0')
        for (u32 count = 0; count < _type.ptr_count; count++)
            printf("%s", type_names[TYPE_NAME_FIRST_PTR_INDEX]);

    /* Printing the type name. */
    if (_type.kind == STRUCT_TYPE) {
        printf("%s", get_struct(_pass, _type.extra_data)->name);
    } else if (IS_TYPE_SPECIAL(_type)) {
        if (_pass != NULLPTR && _pass->data.front_end->type_printer_func \
        != NULLPTR)
            (*_pass->data.front_end->type_printer_func)(_pass, _type);
    } else {
        printf("%s", type_names[_type.kind]);
    }

    /* Printing the after ptrs (if present). */
    if (type_names[TYPE_NAME_LAST_PTR_INDEX][0] != '\0')
        for (u32 count = 0; count < _type.ptr_count; count++)
            printf("%s", type_names[TYPE_NAME_LAST_PTR_INDEX]);

    /* Printing the ending color and '`'. */
    printf("`\x1b[0m");
}

/*
 * This prints the kind of type. If the type is a special type this will call
 * the inputted intermediate pass' type kind printer function if it the
 * intermediate pass is not a NULLPTR and it's type kind printer isn't either.
 */
void print_type_kind(intermediate_pass* _pass, type _type)
{
    // TODO: These escape code colors should be macros in "errors.h".

    /* Printing the inital coloring and '`'. */
    printf("\x1b[090m");

    /* Printing the type kind. */
    if (_type.kind == STRUCT_TYPE) {
        printf("struct");
    } else if (IS_TYPE_SPECIAL(_type)) {
        if (_pass != NULLPTR && _pass->data.front_end->type_printer_func \
        != NULLPTR)
            (*_pass->data.front_end->type_printer_func)(_pass, _type);
    } else {
        _type.ptr_count ? printf("pointer") : printf("type");
    }

    /* Printing the ending color and '`'. */
    printf("\x1b[0m");
}