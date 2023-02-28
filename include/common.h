/*
 * This file contains a lot of commonly used header files.
 */

#ifndef COMMON_H
#define COMMON_H

#define NULLPTR ((void*)0)
#define ARRAY_SIZE(_array) sizeof(_array) / sizeof(_array[0])

#define F32_TO_VOIDPTR(x) ((void*)(*((u32*)(&x))))
#define VOIDPTR_TO_F32(x) (*(f32*)(&x))

#define F64_TO_VOIDPTR(x) ((void*)(*((u64*)(&x))))
#define VOIDPTR_TO_F64(x) (*(f64*)(&x))

#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<errno.h>

#include<defs.h>
#include<typedefs.h>
#include<main.h>
#include<errors.h>
#include<datastructures/vector.h>

#if DEBUG
#include<debug/debug.h>
#endif

#endif