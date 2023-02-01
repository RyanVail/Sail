#ifndef MAIN_H
#define MAIN_H

// TODO: Flags in structs should have toggles here that replace them with bools
// for speed or keep them as flags.

// TODO: There should be a toggle to turn off use counters.

// TODO: There should be a toggle to turn stacks into arrays for speed.

// TODO: Functions should return an ERROR enum instead of a bool to check for
// errors.

// TODO: All mentions of "struct variables" in comments and naming should be
// replaced with "struct members" instead.

// TODO: There should be an optimization pass that replaces ors with ranges like
// if ((u32)x == 0 || (u32)x == 1) can be replaced with if (x.min <= x <= 1)
// and finally if (x <= 1).

/*
 * This will make it so floating point numbers are stored in "void*" during
 * compilatation, which might cause some problems on specific platforms.
 */
#define FLOATS_IN_PTRS 1

/*
 * The "DEBUG" flags makes a lot of runtime errors abort instead of calling
 * "exit(-1)" to allow for backtracing, aswell as compiling with a lot of
 * runtime checks and errors that sadly cannot be in macors.
 */
#define DEBUG 1

/* This flag determines if we store values that are eight bytes long inside void
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


#define FILE_BUFFER_SIZE 4096

#endif