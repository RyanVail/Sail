/*
 * This handles everything that involves types.
 */

#include<types.h>
#include<common.h>
#include<intermediate/struct.h>

/*
 * These are the default type sizes. Corrispond with: { VOID, BOOL, I8, U8, I16,
 * U16, I32, U32, I64, U64, float, double, ptr }
 */
u8 global_type_sizes[STRUCT_TYPE+1] = { 0, 1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 8 };

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
u32 type_get_size(intermediate_pass* _pass, type _type)
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

    /* If this type is special the intermediate pass should handle it. */
    if (IS_TYPE_SPECIAL(_type)) {
        /* Making sure this intermediate pass is valid. */
        if (_pass == NULLPTR || _pass->data.front_end->type_size_func==NULLPTR)
            return __UINT32_MAX__;
        return (*_pass->data.front_end->type_size_func)(_pass, _type);
    }

    /* Returning the size of a ptr if this is a ptr. */
    return _type.ptr_count ? global_type_sizes[TYPE_PTR_INDEX] \
    : global_type_sizes[_type.kind];
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

/*
 * This scales the inputted value to the inputted type. This only works for
 * values no greater magnitude than the maximum value of the type to the power
 * of two minus one. EX. maximum of u8: 256^2-1 = 65535.
 */
i64 scale_value_to_type(i64 value, type _type)
{
    i64 tmp_value = value >> (global_type_sizes[_type.kind] << 3);
    return (tmp_value == 0) ? (value) : (~tmp_value);
}