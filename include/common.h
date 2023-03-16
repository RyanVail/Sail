/*
 * This file contains a lot of commonly used header files.
 */

#ifndef COMMON_H
#define COMMON_H

#define ARRAY_SIZE(_array) sizeof(_array) / sizeof(_array[0])

#define F32_TO_VOIDPTR(x) ((void*)(size_t)(*((u32*)(&x))))
#define VOIDPTR_TO_F32(x) (*(f32*)(&x))

#define F64_TO_VOIDPTR(x) ((void*)(*((u64*)(&x))))
#define VOIDPTR_TO_F64(x) (*(f64*)(&x))

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

#include<defs.h>
#include<typedefs.h>
#include<main.h>
#include<errors.h>
#include<datastructures/vector.h>
#include<debug/debug.h>

#endif