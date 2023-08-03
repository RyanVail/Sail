// TODO: These and other std files need to be fully implemented and these
// macros should be defined with the real macros.
#ifndef _STDIO_H
#define _STDIO_H

#include<typedefs.h>

#ifndef __ssize_t_defined
#define __ssize_t_defined
typedef long int ssize_t;
#endif

// TODO: These need to be done with FILE ptrs.

/* Standard input. */
extern void* stdin;
#define stdin stdin

/* Standard output. */
extern void* stdout;
#define stdout stdout

/* Standard error output. */
extern void* stderr;
#define stderr stderr

/*
 * This is the size of the "printf" buffer. This must be larger than the length
 * of "(null)".
 */
#define PRINTF_BUF_SIZE ((size_t)1024)

// TODO: This shoudl also be called "FILE" I'm pretty sure.
// TODO: These needs more members.
typedef struct File {
    int stream;
} File;

// TODO: i386
// TODO: arm
// TODO: risc-v

// TODO:
void fflush(File* stream);

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
 * This writes count bytes from the inputted buffer to the inputted handle.
 * Returns the number of bytes written.
 */
ssize_t write(int handle, const void* buffer, size_t count);

/*
 * This closes the inputted handle. Returns zero if successful and -1 on
 * errors.
 */
int close(int handle);

/*
 * This attempts to remove the file of the inputted name. Returns a non zero
 * value on failure.
 */
int remove(const char* file_name);

/*
 * This attempts to rename the file of the inputted name to the inputted new
 * name. Returns a non zero value on failure.
 */
int rename(const char* file_name, const char* new_name);

/*
 * This formats the inputted values according to the inputted format to stdout.
 * This returns the number bytes written excluding the null terminating '\0'
 * char.
 */
int printf(const char* format, ...);

/*
 * This formats the inputted values according to the inputted format to the
 * inputted file. This returns the number bytes written excluding the null
 * terminating '\0' char.
 */
int fprintf(File* stream, const char* format, ...);

/*
 * This formats the inputted values according to the inputted format to the
 * inputted file descriptor. This returns the number bytes written excluding
 * the null terminating '\0' char.
 */
int dprintf(int fd, const char* format, ...);

// /*
//  * This function opens the file at the inputted file path with the inputted
//  * open options. Returns NULLPTR if opening the file failed.
//  */
// FILE* fopen(const char* file_path, const char* options);

#endif