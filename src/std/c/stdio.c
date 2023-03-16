#include<std/c/syscall.h>
#include<std/c/stdio.h>
#include<stdio.h>

/*
 * This function opens the file at the inputted file path with the inputted
 * flags. This will return a negative value on errors.
 */
i32 open(const char* file_path, i32 flags)
{
    return syscall(SYS_OPEN, file_path, flags);
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
    return (ssize_t)syscall(SYS_READ, handle, buffer, count);
}

/*
 * This closes the inputted handle. Returns zero if successfull and -1 on
 * errors.
 */
int close(int handle)
{
    return (int)(size_t)syscall(SYS_CLOSE, handle);
}

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