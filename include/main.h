#ifndef MAIN_H
#define MAIN_H

#define DEBUG 1

/* This flag determines if we store values that are 8 bytes long inside void
 * pointers or on the heap. If this flag is set to false it will still compile
 * on 64 bit but will use more memory and call more mallocs.
 */
#define VOID_PTR_64BIT __UINTPTR_MAX__ == 0xffffffffffffffff

#define FILE_BUFFER_SIZE 4096

#endif