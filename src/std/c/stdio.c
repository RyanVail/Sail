// TODO: The C library should be compiled as a library in CMAKE and as such
// the include directory should already be "include/std/c/".

#include<std/c/syscall.h>
#include<std/c/stdio.h>
#include<std/c/stdlib.h>
#include<std/c/string.h>
// #include<stdio.h>

// TODO: These files that have "i32" and other kinds of things should have the
// types that comply with the C standard.

static const char* null_character_string = "(null)";

// TODO: This isn't the best way to do hexadecimal
static const char hex_table[] = "0123456789abcdef";

/*
 * This function opens the file at the inputted file path with the inputted
 * flags. This will return a negative value on errors.
 */
i32 open(const char* file_path, i32 flags)
{
    #if linux && __x86_64__
        return syscall(SYS_OPEN, file_path, flags);
    #endif
}

/*
 * This function opens the file at the inputted file path with the inputted
 * file opening settings as a stream. This will return a negative value on
 * errors. This function is not part of the C standard.
 */
i32 fsopen(const char* file_path, const char* options)
{
    #if linux && __x86_64__
        i32 flags = 0;

        /* This is for compilers that don't support multi char constants. */
        #define U16_CHAR(char0, char1) (char0 | (char1 << 8))

        switch (*(u16*)options)
        {
        case U16_CHAR('r', '\0'):
            flags |= O_RDONLY;
            break;
        case U16_CHAR('w', '\0'):
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case U16_CHAR('a', '\0'):
            flags = O_RDWR | O_CREAT | O_APPEND;
            break;
        case U16_CHAR('r', '+'):
            flags = O_RDWR;
            break;
        case U16_CHAR('w', '+'):
            flags = O_RDWR | O_CREAT | O_TRUNC;
            break;
        case U16_CHAR('a', '+'):
            flags = O_RDWR | O_CREAT | O_APPEND;
            break;
        }
        #undef U16_CHAR

        return syscall(SYS_OPEN, file_path, flags);

    #endif

    return -1;
}

/*
 * This reads count bytes from the inputted handle into the inputted buffer.
 * Returns the number of bytes read.
 */
ssize_t read(int handle, void* buffer, size_t count)
{
    #if linux && __x86_64__
        return (ssize_t)syscall(SYS_READ, handle, buffer, count);
    #endif
}

// TODO: Some times this is called a handle sometimes it's called a file
// descriptor or df.

/*
 * This writes count bytes from the inputted buffer to the inputted handle.
 * Returns the number of bytes written.
 */
ssize_t write(int handle, const void* buffer, size_t count)
{
    #if linux && __x86_64__
        return (ssize_t)syscall(SYS_WRITE, handle, buffer, count);
    #endif
}

/*
 * This closes the inputted handle. Returns a non zero value on failure.
 */
int close(int handle)
{
    #if linux && __x86_64__
        return (int)(size_t)syscall(SYS_CLOSE, handle);
    #endif
}

/*
 * This attempts to remove the file of the inputted name. Returns a non zero
 * value on failure.
 */
int remove(const char* file_name)
{
    #if linux && __x86_64__
        return (int)(size_t)syscall(SYS_UNLINK, file_name);
    #endif
}

/*
 * This attempts to rename the file of the inputted name to the inputted new
 * name. Returns a non zero value on failure.
 */
int rename(const char* file_name, const char* new_name)
{
    #if linux && __x86_64__
        return (int)(size_t)syscall(SYS_RENAME, file_name, new_name);
    #endif
}

/*
 * This prints the inputted character string to the inputted file descriptor
 * and returns the number of bytes printed.
 */
static inline int __fd_print_character_string__(int fd, const char* str)
{
    /* Printing "(null)" if the character string is a NULLPTR. */
    if (str == NULLPTR)
        return (int)write (
            fd,
            &null_character_string,
            sizeof(null_character_string)
        );

    return (int)write(fd, &str, strlen(str));
}

/*
 * This prints the inputted character string to the inputted buffer and returns
 * the number of characters printed.z
 */
static inline int __print_character_string_to_buffer(char* buffer,
const char* str)
{
    /* Writing "(null)" if the character string is a NULLPTR. */
    if (str == NULLPTR) {
        strcpy(buffer, null_character_string);
        return (int)sizeof(null_character_string);
    }

    strcpy(buffer, str);
    return (int)strlen(str);
}

// TODO: The below functions should be inlined.
u32 __print_unsigned_decimal_to_buffer__(char* buffer, u32 value)
{
    // TODO: THIS NEEDS TO USE memrchr BECAUSE RIGHT NOW IT WILL NOT WORK IN A
    // LOT OF CASSES.
    /*
     * The below code doesn't handle a value of zero so it has to be hard coded
     * up here.
     */
    if (value == 0) {
        *buffer = '0';
        return 1;
    }

    /* Writing the decimal number in reverse to the buffer. */
    buffer += (size_t)9;
    for (u32 i=0; i < 10; i++) {
        *buffer = (value % 10) + '0';
        value /= 10;
        buffer--;
    }

    buffer++;

    /* Finding the first non zero digit of the number. */
    char* non_zero = strrchr(buffer, '0') + (size_t)1;

    // TODO: Handle this.
    if (non_zero == NULLPTR + (size_t)1)
        non_zero = buffer;

    char* buffer_end = buffer + (size_t)9;
    u32 number_size = (u32)((size_t)buffer_end - (size_t)non_zero + (size_t)1);

    /*
     * Going from the end of the buffer to the first non zero digit placing the
     * digits in the front of the buffer.
     */
    for (; non_zero <= buffer_end; non_zero++, buffer++)
        *buffer = *non_zero;

    return number_size;
}

u32 __print_octal_to_buffer__(char* buffer, u32 value)
{
    u32 zeros = 0;

    /*
     * Because octal doesn't fit nicely into a 32 bit int, (32/3) = 10 R2, this
     * checks if the first two bits are not zero and if they aren't this prints
     * them and skips to the printing of the rest of the value.
     */
    u32 tmp_value = value;
    value <<= 2;
    if (tmp_value & (0b11 << 30))
        goto print_octal_write_value_label;
    else
        zeros++;

    /* Skipping the first zeros of the value. */
    for (; zeros < 11; zeros++) {
        if (value & (0x7 << 29))
            break;
        value <<= 3;
    }

    print_octal_write_value_label: ;

    /* Writing the rest of the value in octal. */
    for (u32 i=0; i < 11 - zeros; i++) {
        printf("%o\n", value);
        buffer[i] = (((value & (0x7 << 29))) >> 29) + '0';
        value <<= 3;
    }

    return 11 - zeros;
}

u32 __print_hex_to_buffer__(char* buffer, u32 value)
{
    /* Skipping the first zeros of the value. */
    u32 zeros;
    for (zeros=0; zeros < 8; zeros++) {
        if (value & (0xf << 28))
            break;
        value <<= 4;
    }

    /* Writing the rest of the value in hexadecimal. */
    for (u32 i=0; i < 8 - zeros; i++) {
        buffer[i] = hex_table[(value & (0xf << 28)) >> 28];
        value <<= 4;
    }

    return 8 - zeros;
}

/*
 * This prints the inputted value in hexadecimal to the inputted file
 * descriptor using the inputted buffer and returns the number of bytes written.
 */
static inline int __fd_print_hex__(int fd, char* buffer, unsigned int value)
{
    return (int)write(fd, buffer, __print_hex_to_buffer__(buffer, value));
}

typedef enum print_format_flag {
    PRINT_FORMAT_ALWAYS_SIGNED = 2,
    PRINT_FORMAT_SPACE_PREFIX_FOR_NO_SIGN = 4,
} print_format_flag;

typedef enum print_format_type {
    PRINT_FORMAT_CHARACTER_STRING,
} print_format_type;

typedef struct print_format {
    print_format_flag flags;
    print_format_type type;
    u32 digits;
    u32 spaces;
} print_format;

// TODO: These
// /*
//  * This formats the inputted values according to the inputted format to stdout.
//  * This returns the number bytes written excluding the null terminating '\0'
//  * char.
//  */
// int printf(const char* format, ...)
// {
//     // dprintf(stdout, format, ...); // TODO: This needs to use the dynamic
//                                      // params thing.
// }

// /*
//  * This formats the inputted values according to the inputted format to the
//  * inputted file. This returns the number bytes written excluding the null
//  * terminating '\0' char.
//  */
// int fprintf(File* stream, const char* format, ...)
// {
//     // TODO:
// }

// /*
//  * This formats the inputted values according to the inputted format to the
//  * inputted file descriptor. This returns the number bytes written excluding
//  * the null terminating '\0' char.
//  */
// int dprintf(int fd, const char* format, ...)
// {
//     // TODO:
// }

// /*
//  * This function opens the file at the inputted file path with the inputted
//  * open options. Returns NULLPTR if opening the file failed.
//  */
// FILE* fopen(const char* file_path, const char* options)
// {
//     #if linux && __x86_64__
//         i32 flags = 0;
//         FILE* _file;

//         /* Initing the file. */
//         _file = malloc(sizeof(FILE));
//         if (_file == NULLPTR)
//             return NULLPTR;
//         memset(_file, 0, sizeof(FILE));

//         /* Initing the file lock. */
//         _file->lock = malloc(sizeof(file_lock));
//         if (_file->lock == NULLPTR)
//             return NULLPTR;
//         memset(_file->lock, 0, sizeof(file_lock));

//         /* This is for compilers that don't support multi char constants. */
//         #define U16_CHAR(char0, char1) (char0 | (char1 << 8))

//         switch (*(u16*)options)
//         {
//         case U16_CHAR('r', '\0'):
//             flags |= O_RDONLY;
//             break;
//         case U16_CHAR('w', '\0'):
//             flags = O_WRONLY | O_CREAT | O_TRUNC;
//             break;
//         case U16_CHAR('a', '\0'):
//             flags = O_RDWR | O_CREAT | O_APPEND;
//             break;
//         case U16_CHAR('r', '+'):
//             flags = O_RDWR;
//             break;
//         case U16_CHAR('w', '+'):
//             flags = O_RDWR | O_CREAT | O_TRUNC;
//             break;
//         case U16_CHAR('a', '+'):
//             flags = O_RDWR | O_CREAT | O_APPEND;
//             break;
//         }

//         #undef U16_CHAR

//         i32 descriptor = syscall(SYS_OPEN, file_path, flags);

//         if (descriptor < 0)
//             return NULLPTR;

//         _file->file_descriptor = descriptor;
//         return _file;

//     #endif

//     return NULLPTR;
// }