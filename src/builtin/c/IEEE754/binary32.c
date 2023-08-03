/*
 * This is a software implementation of the IEEE754 single precision (binary 32)
 * floating point numbers.
 */

#define DONT_DEFINE_FLOAT_TYPES 1
#include<typedefs.h>
#include<stdbool.h>

// TODO: This should be typedefed to "__builtin_f32" if compiling with sail when
// that works.
typedef u32 f32;

#define EXPONENT_BIT_MASK 0b1111111100000000000000000000000
#define MANTISSA_BIT_MASK 0b11111111111111111111111

// TODO: 0xFF is used sometimes other times 0xff.
#define F32_IS_NEGATIVE(_f32) ((_f32) & (1 << 31))
#define F32_GET_EXPONENT(_f32) (((_f32) & (0xff << 23)) >> 23)
#define F32_GET_MANTISSA(_f32) ((_f32) & MANTISSA_BIT_MASK)

// TODO: I don't think these are implemeted correctly
/* This returns true if the inputted binary32 is not a number. */
bool __builtin_binary32_is_nan(f32 _f32)
{
    return !((_f32) ^ (0b1111111 << 23));
}

/* This returns true if the inputted binary32 is an infinite. */
bool __builtin_binary32_is_inf(f32 _f32)
{
    return !((_f32) ^ (0b11111111 << 22));
}

/* This returns true if the inputted binary32 is a finite number. */
bool __builtin_binary32_is_finite(f32 _f32)
{
    return !(__builtin_binary32_is_inf(_f32))
        && !(__builtin_binary32_is_nan(_f32));
}

// TODO: More functions are needed with different rounding.
/*
 * This converts the inputted u32 into a binary32 float, rounding toward
 * even mantissa values on ties.
 */
f32 __builtin_binary32_convert_from_u32(u32 _u32)
{
	/* The below code breaks on a value of zero. */
    if (_u32 == 0)
		return 0;

    /* Adding the exponent. */
	u32 num_start = __builtin_clz(_u32);
	f32 result = (127 + 31 - num_start) << 23;

	/* Adding the mantissa. If precision is being lost converting into a float
     * resulting float has to be rounded.
     */
	if (num_start < 8) {
		result |= (_u32 >> 8 - num_start) & MANTISSA_BIT_MASK;

        /* This is the extra digits that get rounded away. */
		u32 extra = (_u32 & ((1 << 8 - num_start) - 1));

        /*
         * This is the half way point between the extra digits and a digit that
         * made it into the float.
         */
		u32 half_extra = (1 << 7 - num_start);

        /* Adding another one to the value if the extra is over the half. */
		result += extra > half_extra;

        /* Rounding to the nearest even when the extra is half. */
		result += (extra == half_extra) && (result & 1);
	} else {
		result |= (_u32 << (num_start - 8)) & MANTISSA_BIT_MASK;
	}

    /* Returning the result. */
    return result;
}

/*
 * This converts the inputted i32 into a binary32 float, rounding toward
 * even mantissa values on ties.
 */
f32 __builtin_binary32_convert_from_i32(i32 _i32)
{
    /* The code below breaks on a value of zero. */
    if (_i32 == 0)
		return 0;

    /* Taking the negative away from the value and putting it into a bool. */
    bool negative = _i32 & (1 << 31);

    /* Turning the number into a u32. */
    u32 _u32 = (negative) ? -_i32 : _i32;

    /* Turning the u32 into an f32. */
    f32 result = __builtin_binary32_convert_from_u32(_u32);

    /* Adding back the negative and returning the value. */
    result |= (negative << 31);
    return result;
}

// TODO: Document this more.
/*
 * This scales the inputted binary32 floating point number's mantissa to match
 * its value in the inputted exp.
 */
u32 __binary32_scale_to(f32* _f32, u32 exp)
{
    /* Getting the exponent of the target float. */
    u32 _exp = F32_GET_EXPONENT(*_f32);

    i32 exp_delta = (i32)exp - (i32)_exp;

    /*
     * The below code will not work if the exponent delta is larger than the
     * size of the mantissa.
     */
    if (exp_delta >= 24) {
        *_f32 = 0;
        return exp_delta;
    }

    // TODO: This really doesn't need to copy the exponent.
    *_f32 = (
        /* Copying the exponent. */
        (exp << 23)

        /* Shifting the mantissa for the new exponent. */
        | (F32_GET_MANTISSA(*_f32) >> (exp_delta))

        /* Adding back the floats exponenet as a scalled matissa value. */
        + (1 << (23 - (exp_delta)))
    );

    return exp_delta;
}

// TODO: This should return a flag for rounding.
// TODO: Does this need to return the exp dif?
/*
 * This scales the two inputted binary32 floating point numbers to have the
 * exponent of the larger number.
 */
u32 __binary32_scale_to_larger(f32* f32_0, f32* f32_1)
{
    /* Getting the exponenet of the two floats. */
    u32 exp0 = F32_GET_EXPONENT(*f32_0);
    u32 exp1 = F32_GET_EXPONENT(*f32_1);

    /* Scaling the float with the smaller exponent. */
    if (exp0 > exp1)
        return __binary32_scale_to(f32_1, exp0);
    else if (exp0 < exp1)
        return __binary32_scale_to(f32_0, exp1);
}

/* This function adds the inputted floats assuming both are positive. */
f32 __builtin_binary32_addition_base(f32 f32_0, f32 f32_1)
{
    printf("%f\n", *(float*)&f32_1);
    printf("%f\n", *(float*)&f32_0);
    /* Scaling the floats to have the same exponent. */
    __binary32_scale_to_larger(&f32_0, &f32_1);
    printf("=> %f\n", *(float*)&f32_1);
    printf("=> %f\n", *(float*)&f32_0);

    u32 mantissa_result = F32_GET_MANTISSA(f32_0) + F32_GET_MANTISSA(f32_1);

    /*
     * If both numbers don't have a mantissa, which because of the scaling means
     * they're the same number the number's exponent is incramented and returned
     * to multiply the number by two.
     */
    if (mantissa_result == 0)
        return f32_0 + (1 << 23);

    /*
     * If the mantissa result is lands exactly on a power of two the exponent is
     * incramented and the mantissa is thrown out.
     */
    if (mantissa_result == (MANTISSA_BIT_MASK + 1)) {
        f32_0 += (1 << 23);

    /* If the number needs another exponent. */
    } else if (mantissa_result > MANTISSA_BIT_MASK) {
        /* Incramenting the exponent. */
        f32_0 += (1 << 23);

        /*
         * Cutting the extra bits off the mantissa result after shifiting it to
         * be within the newely incramented exponent.
         */
        mantissa_result = F32_GET_MANTISSA(mantissa_result >> 1);
    }

    // TODO: adding the the exponenet should have a function to handle over
    // overflows.

    return (f32_0 & (0xFF << 23)) | mantissa_result;
}

/*
 * This function subtracts the first inputted float by the second inputted float
 * this ignores signs.
 */
f32 __builtin_binary32_subtraction_base(f32 f32_0, f32 f32_1)
{
    printf("=== %f\n", *(float*)&f32_0);
    printf("=== %f\n", *(float*)&f32_1);
    /* Scaling the floats to have the same exponent. */
    u32 exp = __binary32_scale_to_larger(&f32_0, &f32_1);
    printf("==> %f\n", *(float*)&f32_0);
    printf("==> %f\n", *(float*)&f32_1);

    i32 mantissa_result;

    /*
     * If both numbers don't have a mantissa, which because of the scaling means
     * they're the same so zero is returned.
     */
    if (mantissa_result == 0)
        return 0;

    /*
     * If the result is going to be negative numbers have to carried from the
     * exponent into the mantissa.
     */
    if (mantissa_result < 0) {
        if (!((f32_0 & 0xff) << 23)) {
            mantissa_result = -mantissa_result;
            u32 result = 24 - (32 - __builtin_clz(mantissa_result));
            printf("%u\n", result);
            // mantissa_result >>= result;
            // mantissa_result = F32_GET_MANTISSA(mantissa_result);
            // mantissa_result = 0;
            // mantissa_result += (MANTISSA_BIT_MASK+1);
            f32_0 |= (1 << 31);
            f32_0 -= (result << 23);
        } else {
            f32_0 -= (1 << 23);
            mantissa_result <<= 1;
            mantissa_result += (MANTISSA_BIT_MASK+1);
        }
    }

    // TODO: adding the the exponenet should have a function to handle over
    // overflows.
    return (f32_0 & (1 << 31)) | (f32_0 & (0xFF << 23)) | mantissa_result;
}

// TODO:
f32 __builtin_binary32_subtraction(f32 f32_0, f32 f32_1)
{
    /* The code below breaks on a value of zero. */
    if (f32_0 == 0)
        return f32_1;
    if (f32_1 == 0)
        return f32_0;

    /*
     * If both of these floats are negative or both of them are positive this is
     * handled an addition operation.
     */
    if (F32_IS_NEGATIVE(f32_1) == F32_IS_NEGATIVE(f32_0))
        return __builtin_binary32_addition_base(f32_0, f32_1);

    /* Doing and returning the subtraction operation. */
    return (F32_IS_NEGATIVE(f32_1))
        ? __builtin_binary32_subtraction_base(f32_0, f32_1)
        : __builtin_binary32_subtraction_base(f32_1, f32_0);
}

// TODO: Comment
// TODO: This doesn't handle negatives.
f32 __builtin_binary32_addition(f32 f32_0, f32 f32_1)
{
    // TODO: Clean this mess up.

    /* The code below breaks on a value of zero. */
    if (f32_0 == 0)
        return f32_1;
    if (f32_1 == 0)
        return f32_0;

    /*
     * This code below only works on adding two positive floats or two negative
     * floats so this handles subtraction.
     */
    if (F32_IS_NEGATIVE(f32_0) && !F32_IS_NEGATIVE(f32_1))
        return __builtin_binary32_subtraction_base(f32_1, f32_0);
    else if (F32_IS_NEGATIVE(f32_1) && !F32_IS_NEGATIVE(f32_0))
        return __builtin_binary32_subtraction_base(f32_0, f32_1);

    /* Doing and returning the addition operation. */
    return __builtin_binary32_addition_base(f32_0, f32_1);
}

int main()
{
    // -1073762467
    // f32 a = __builtin_binary32_convert_from_i32(-1073762467);
    // printf("%f\n", *(float*)&a);
    // return 0;

    f32 a = __builtin_binary32_convert_from_u32(3);
    f32 b = __builtin_binary32_convert_from_u32(16777216);
    f32 c = __builtin_binary32_addition(a, b);

    // a += (b & MANTISSA_BIT_MASK);
    // printf("%f\n", *(float*)&b);
    printf("%f\n", *(float*)&c);
    // for (i32 v=0; v > INT32_MIN; v--) {
    //     u32 a = __builtin_binary32_convert_from_i32(v);
    //     if (*(float*)&a != (float)v)
    //         printf("Failed %i %f : %f\n", v, *(float*)&a, (float)v);
    // }
}