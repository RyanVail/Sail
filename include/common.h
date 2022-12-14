/*
 * This file contains a lot of commonly used header files.
 */

#ifndef COMMON_H
#define COMMON_H

#define ARRAY_SIZE(_array) sizeof(_array) / sizeof(_array[0])

#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>

#include<typedefs.h>
#include<errors.h>
#include<main.h>
#include<datastructures/vector.h>

/*
 * This takes in a string an convertes the english characters to lower case.
 */
void strlower(char* _string);

#endif