/*
 * This file contains a lot of commonly used header files.
 */
// TODO: Replace all __TYPE_MAX__ in the documentation.
// TODO: Datastructures should be renamed Utils.
#ifndef COMMON_H
#define COMMON_H

#define ARRAY_SIZE(_array) sizeof(_array) / sizeof(_array[0])

#define F32_TO_VOIDPTR(x) ((void*)(size_t)(*((u32*)(&x))))
#define VOIDPTR_TO_F32(x) (*(f32*)(&x))

#define F64_TO_VOIDPTR(x) ((void*)(*((u64*)(&x))))
#define VOIDPTR_TO_F64(x) (*(f64*)(&x))

#if COMPILER_HAS_UNREACHABLE && !DEBUG
    #define UNREACHABLE() \
        __builtin_unreachable()
#else
    #define UNREACHABLE() \
        send_error("Reached unreachable scope"); \
        __builtin_unreachable()
#endif

#include<main.h>

#if USE_INTERNAL_STD
    #include<std/c/stdlib.h>
    #include<std/c/stdio.h>
    #include<std/c/stdbool.h>
    #include<std/c/string.h>

    // TODO:
    #include<stdio.h>
    #include<stdlib.h>
    #include<errno.h>
#else
    #include<stdlib.h>
    #include<stdlib.h>
    #include<stdio.h>
    #include<stdbool.h>
    #include<string.h>
    #include<errno.h>
#endif
// TODO: Implement the rest of these files in the C std

#include<typedefs.h>
#include<main.h>
#include<errors.h>
#include<datastructures/vector.h>
#include<debug/debug.h>

#endif