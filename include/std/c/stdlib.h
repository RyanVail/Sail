#ifndef STD_C_STDLIB_H
#define STD_C_STDLIB_H

#include<typedefs.h>
#include<stddef.h>

/* Malloc settings */
#define PAGE_GROWTH_MOD_BYTE_SIZE (4096)
#define MMAP_THREASHOLD (128 * 1024)

/* This preforms a program termiantion with the inputted error code. */
void exit(i32 error_code);

/* This preforms a termination of the program without doing any clean up. */
void abort();

/*
 * TODO: This.
 */
void free(void* ptr);

// TODO: This.
void* realloc(void* ptr, size_t bytes);

/*
 * This allocated bytes bytes of memory on the heap and returns a ptr to the
 * allocated memory. In cases of failure this will return NULLPTR.
 */
void* malloc(size_t bytes);

#if __SAIL_MALLOC_DANGLING_FUNCTIONS__
/*
 * This function goes through all of the memory allocations in malloc and
 * returns the number of bytes stored in dangling ptrs. This function is not
 * part of the C standard.
 */
size_t __count_dangling_bytes__();

/*
 * This function goes through all of the memory allocations in malloc and
 * returns the number of dangling ptrs. This function is not part of the C
 * standard.
 */
u32 __count_dangling_ptrs__();
#endif

#endif