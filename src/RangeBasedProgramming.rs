// These type defs and constants are generated during the compilation of the
// compiler to the "std/{TODO:}/defs.{TODO:}".

// Signed int definitions
typedef i32 -2147483648..2147483648;
typedef i16 -32768..32767;
typedef i8 -128..127;

// Unsigned int definitions
typedef u32 0..4294967295
typedef u16 0..65535
typedef u8 0..255

// Bool definitions
typedef bool [0,1];
const true: bool = 1;
const false: bool = 0;

// Float definitions
typedef f32 {_f};
const f32_min: f32 = 0.00000000000000000000000000000000000001175494350822287507\
9687365372222456778186655567720875215087517062784172594547271728515625;
const f32_max: f32 = 340282346638528859811704183484516925440

typedef even {i32,^1};
typedef odd {i32,&1};

/*
 * This adds the inputted even (a) and the inputted odd (b) numbers together and
 * will always result in an odd number.
 */
fn even_odd_i32_adder(a: even, b: odd) -> odd
{
    /*
     * If this cannot be asserted during compile time this will be a run time
     * check and error.
     */
    return odd(i32(a) + i32(b));
}

// Anything wrapped in a test scope will not be present in other than test mode.
// test's fail if the returned bool is false otherwise they pass.
test "even + odd = odd" {
    return (@TypeOf(even + odd) == odd);
};

// '@' functions are built into the compiler and as seen from above and below
// can be used to gather compile time information about a type.
test "even odd equal logical sizes" {
    return (@bitSizeOf(even) == @bitSizeOf(odd));
};

// Bit casting can be used for optimizations but are undefined behaviour since
// they can cause odd program execution and different results on optimization
// levels.
test "unsafe non error checked bit casting" {
    a: even = 4;
    b: odd = 5;
    return (@TypeOf(@bitCast(even, b)) == even);
}

// Invalid type states can be checked during compilation time and during
// runtime. In the below case the types will always be invalid.
test "unsafe type state checking" {
    a: odd = 5;
    return (@typeValid(@bitCast(even, b)));
}