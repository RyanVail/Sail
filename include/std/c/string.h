#ifndef _STRING_H
#define _STRING_H

#include<stddef.h>
#include<typedefs.h>

/*
 * This copies len bytes from the inputted source ptr into the inputted
 * destination ptr. This returns a ptr to the destination.
 */
void* memcpy(void* destination, const void* source, size_t len);

// TODO: memmove.

/*
 * This copies the source string into the destination string including the null
 * terminating '\0' char. This returns a ptr to the destination. This doesn't
 * do any bounds checking.
 */
char* strcpy(char* destination, const char* source);

/*
 * This copies no more than n chars into the destination string from the source
 * string this will copy the null terminating char if it's within n chars.
 */
char* strncpy(char* destination, const char* source, size_t n);

/*
 * This adds the source string onto the end of the destination string overiding
 * the null terminating '\0' char in the destination and copying the null
 * terminating '\0' from the source string. This returns a ptr to the start of
 * the destination string. This doesn't do any bounds checks.
 */
char* strcat(char* destination, const char* source);

/*
 * This compares len bytes of the two inputted ptrs. Returns zero if the
 * ptr's values are equal, a negative value if the value of the first ptr is
 * less than the second's value, and a positive value if the value of the first
 * ptr is greater than the second.
 */
i32 memcmp(const void* ptr0, const void* ptr1, size_t len);

/*
 * This compares the two inputted strings. Returns zero if the strings are
 * equal, a negative value if the first string is less than the second, and a
 * positive value if the first string is greater than the second.
 */
i32 strcmp(const char* string0, const char* string1);

// TODO: strcoll.

/*
 * This compares no more than n chars of the two inputted strings. Returns zero
 * if the strings are equal, a negative value if the first string is less than
 * the second, and a positive value if the first string is greater than the
 * second.
 */
i32 strncmp(const char* string0, const char* string1, size_t n);

// TODO: strxfrm.

/*
 * This returns the first occurrence of the inputted c as a i8 in the first n
 * bytes of the inputted source. This returns a ptr to the found index of the
 * inputted byte or NULLPTR if it isn't found.
 */
void* memchr(const void* source, i32 c, size_t n);

/*
 * This finds the first occurrence of the char of c in the inputted source
 * string and returns a ptr to it or NULLPTR if it isn't found.
 */
char* strchr(const char* source, int c);

/*
 * This computes the maximum length of the inital segment of the first string
 * that doesn't contain any chars the second string contains. This doesn't
 * include the null terminating '\0' char.
 */
size_t strcspn(const char* string0, const char* string1);

/*
 * This locates the first occurrence in the first inputted string of any char
 * in the second string or a NULLPTR if it isn't found. 
 */
char* strpbrk(const char* string0, const char* string1);

/*
 * This returns the last occurrence of the inputted c as a char in the inputted
 * string the null terminating '\0' char is considered part of the string. If
 * the inputted c as a char isn't found NULLPTR is returned.
 */
char* strrchr(const char* string, int c);

/*
 * This returns the length of the maximum inital segment of the first inputted
 * string that consists entirely of chars from the second inputted string.
 */
size_t strspn(const char* string0, const char* string1);

/*
 * This locates the first occurrence of the second inputted string in the first
 * inputted string excluding the null terminating '\0' char. This will return a
 * ptr to the first char of the found segment in the first inputted string.
 */
char* strstr(const char* string0, const char* string1);

// TODO: strtok

/* This sets n bytes of destination to c converted to a u8. */
void* memset(void* destination, int c, size_t n);

// TODO: strerror

/*
 * This counts and returns the number of chars in the inputted string. The
 * length of the string doesn't include the null termination '\0' char.
 */
size_t strlen(const char* string);

#endif
