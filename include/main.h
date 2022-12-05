#ifndef MAIN_H
#define MAIN_H

/*
 * The "DEBUG" flags makes a lot of runtime errors abort instead of calling
 * "exit(-1)" to allow for backtracing, aswell as compiling with a lot of
 * runtime checks and errors that sadly cannot be in macors.
 */
#define DEBUG 1

/* This flag determines if we store values that are 8 bytes long inside void
 * pointers or on the heap. If this flag is set to false it will still compile
 * on 64 bit but will use more memory and call more mallocs.
 */
#ifdef _WIN32
	#if _WIN64 
		#define VOID_PTR_64BIT 1
	#else
		#define VOID_PTR_64BIT 0
	#endif
#else
#define VOID_PTR_64BIT (__UINTPTR_MAX__ == 0xffffffffffffffff)
// #define VOID_PTR_64BIT 0 // This is used for testing
#endif

#define FILE_BUFFER_SIZE 4096

#endif