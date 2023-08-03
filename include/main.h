#ifndef MAIN_H
#define MAIN_H

// TODO: Flags in structs should have toggles here that replace them with bools
// for speed or keep them as flags.

// TODO: There should be a toggle to turn off use counters.

// TODO: There should be a toggle to turn stacks into arrays for speed.

// TODO: Functions should return an ERROR enum instead of a bool to check for
// errors.

// TODO: There should be a function or macro called "vector_top"

// TODO: All mentions of "struct variables" in comments and naming should be
// replaced with "struct members" instead.

// TODO: A few strings are too big to fit into the 80 char line limit.

// TODO: Some structs have "contents" other have "content".

// TODO: "if (bool) { value++; }" can be optimized as "value += bool;".

// TODO: Add unit tests to the cli.
/*
 * This flag will make it so the compiler will both compile with unit tests and
 * a runtime flag passed into the program to preform the unit tests.
 */
#ifndef UNIT_TESTS
	#define UNIT_TESTS 0
#endif

/*
 * This will make it so unreachable scopes within the compiler are compiled with
 * calls to "__builtin_unreachabe()" which some compiler might not have.
 */
#ifndef COMPILER_HAS_UNREACHABLE
	#define COMPILER_HAS_UNREACHABLE 1
#endif

/*
 * This flag controls if this should compile using the internal standard C
 * library or not. This doesn't stop the standard C library from being included
 * that has to be done externally. The C library included with Sail is not meant
 * to be fast or efficent, it is just to one day make it self hosting.
 */
#ifndef USE_INTERNAL_STD
	#define USE_INTERNAL_STD 1
#endif

/*
 * This flag controls if there should be more descriptive errors. To get more
 * descriptive errors this will store another copy of the file alongside the
 * tokenized version that holds the lines, line index, and files tokens come
 * from.
 */
#ifndef DESCRIPTIVE_ERRORS
	#define DESCRIPTIVE_ERRORS 1
#endif

// TODO: It would make more sense for this to be a flag while compiling and
// selected by front ends.
/*
 * This flag controls if the compiler should be compiled with runtime checks
 * these runtime checks are optional during compilation but will allow the
 * runtime to do analysis of predefined array bounds assuming the front end sets
 * the array's bounds.
 */
#ifndef RUNTIME_ARRAY_BOUND_CHECKS
	#define RUNTIME_ARRAY_BOUND_CHECKS 1
#endif

/*
 * This flag changes if the function "get_lowest_type" uses the predefined C
 * macros for type sizes EX. UINT8_MAX or uses the type sizes to
 * dynamically generate the maximums and minimums of a given type. If this is
 * set to true it cause problems for platforms in which type sizes are non
 * normal and if the type names are changed.
 */
#ifndef USE_PREDEF_TYPE_MAXES
	#define USE_PREDEF_TYPE_MAXES 1
#endif

/*
 * This flag turns on index checking for vectors and also makes sure a vector
 * is initted before operation on it.
 */
#ifndef VECTOR_CHECK_BOUNDS
	#define VECTOR_CHECK_BOUNDS 1
#endif

/* This turns on checks after mallocing memory and other heap operations. */
#ifndef CHECK_MALLOC_RETURNS
	#define CHECK_MALLOC_RETURNS 1
#endif

/*
 * This will make it so floating point numbers are stored in voidptrs during
 * compilation, which might cause some problems on specific platforms.
 */
#ifndef FLOATS_IN_PTRS
	#define FLOATS_IN_PTRS 1
#endif

/*
 * The "DEBUG" flags makes a lot of runtime errors abort instead of calling
 * "exit(-1)" to allow for backtracing, as well as compiling with a lot of
 * runtime checks and errors that sadly cannot be in macros.
 */
#ifndef DEBUG
	#define DEBUG 0
#endif

/*
 * This flag determines if we store values that are eight bytes long inside void
 * pointers or on the heap. If this flag is set to false it will still compile
 * on 64 bit but will use more memory and call more mallocs.
 */
#ifdef _WIN32
	#if _WIN64 
		#define PTRS_ARE_64BIT 1
	#else
		#define PTRS_ARE_64BIT 0
	#endif
#else
	#define PTRS_ARE_64BIT (UINTPTR_MAX == 0xffffffffffffffff)
	// #define PTRS_ARE_64BIT 0
#endif

/* Flags in "tokenizer.c". */
#define TOKENIZER_FILE_BUFFER_SIZE 4096
#define TOKENIZER_TOKEN_BUFFER_SIZE 512

#if DESCRIPTIVE_ERRORS
	#define TOKENIZER_SOURCE_LINE_BUFFER_SIZE 512
#endif

#define SALMON_PARSING_ERROR_PRE_TOKENS 5
#define SALMON_PARSING_ERROR_POST_TOKENS 5

#endif