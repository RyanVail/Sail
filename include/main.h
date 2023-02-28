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

// TODO: There should be an optimization pass that replaces ors with ranges like
// if ((u32)x == 0 || (u32)x == 1) can be replaced with if (x.min <= x <= 1)
// and finally if (x <= 1).

// TODO: Some structs have "contents" other have "content".

/*
 * This flag controls if there should be more descriptive errors. To get more
 * descriptive errors this will store another copy of the file alongside the
 * tokenized version that holds the lines, line index, and files tokens come
 * from.
 */
#define DESCRIPTIVE_ERRORS 1

/*
 * This flag changes if the function "get_lowest_type" uses the predefined C
 * macros for type sizes EX. "__UINT8_MAX__" or uses the type sizes to
 * dynamically generate the maximums and minimums of a given type. If this is
 * set to true it cause problems for platforms in which type sizes are non
 * normal and if the type names are changed.
 */
#define USE_PREDEF_TYPE_MAXES 1

/*
 * This flag turns on index checking for vectors and also makes sure a vector
 * is initted before operation on it.
 */
#define VECTOR_CHECK_BOUNDS 1

/* This turns on checks after mallocing memory and other heap operations. */
#define CHECK_MALLOC_RETURNS 1

/*
 * This will make it so floating point numbers are stored in voidptrs during
 * compilation, which might cause some problems on specific platforms.
 */
#define FLOATS_IN_PTRS 1

/*
 * The "DEBUG" flags makes a lot of runtime errors abort instead of calling
 * "exit(-1)" to allow for backtracing, as well as compiling with a lot of
 * runtime checks and errors that sadly cannot be in macros.
 */
#define DEBUG 1

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
	#define PTRS_ARE_64BIT (__UINTPTR_MAX__ == 0xffffffffffffffff)
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