// TODO: These and other std files need to be fully implemented and these
// macros should be defined with the real macros.
#ifndef STD_C_STDIO_H
#define STD_C_STDIO_H

#include<typedefs.h>

#ifndef __ssize_t_defined
#define __ssize_t_defined
typedef long int ssize_t;
#endif

// TODO: i386
// TODO: arm
// TODO: risc-v

// #if linux

// typedef struct FILE FILE;
// typedef struct file_buffer_index file_buffer_index;

// typedef struct file_buffer_index {
//   file_buffer_index* next;
//   FILE* buffer;
//   i32 index;
// } file_buffer_index;

// typedef struct file_lock {
//     i32 lock;
//     i32 c;
//     void* parent;
// } file_lock;

// typedef struct FILE {
//     i32 flags_0;
//     char* read_ptr;
//     char* read_end;
//     char* read_base;
//     char* write_base;
//     char* write_ptr;
//     char* write_end;
//     char* buf_base;
//     char* buf_end;
//     char* save_base;
//     char* backup_base;
//     char* save_end;
//     file_buffer_index* indexes;
//     FILE* next;
//     i32 file_descriptor;
//     i32 flags_1;
//     long notused;
//     u8 column;
//     i8 vtable_offset;
//     char buf;
//     file_lock* lock;
// } FILE;
// #endif

/*
 * This function opens the file at the inputted file path with the inputted
 * flags. This will return a negative value on errors.
 */
i32 open(const char* file_path, i32 flags);

/*
 * This function opens the file at the inputted file path with the inputted
 * file opening settings as a stream. This will return a negative value on
 * errors. This function is not part of the C standard.
 */
i32 fsopen(const char* file_path, const char* options);

/*
 * This reads count bytes from the inputted handle into the inputted buffer.
 * Returns the number of bytes read.
 */
ssize_t read(int handle, void* buffer, size_t count);

/*
 * This closes the inputted handle. Returns zero if successfull and -1 on
 * errors.
 */
int close(int handle);

// /*
//  * This function opens the file at the inputted file path with the inputted
//  * open options. Returns NULLPTR if opening the file failed.
//  */
// FILE* fopen(const char* file_path, const char* options);

#endif