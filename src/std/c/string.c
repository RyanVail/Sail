#include<std/c/string.h>

/*
 * This copyies len bytes from the inputted source ptr into the inputted
 * destination ptr. This returns a ptr to the destination.
 */
void* memcpy(void* destination, const void* source, size_t len)
{
    u8* _destination = destination;
    const u8* _source = source;

    while (len > 0) {
        *_destination = *_source;
        _destination++;
        _source++;
        len--;
    }

    return destination;
}

/*
 * This copies the source string into the destination string including the null
 * terminating '\0' char. This returns a ptr to the destination. This doesn't
 * do any bounds checking.
 */
char* strcpy(char* destination, const char* source)
{
    char* _destination = destination;

    while (*source != '\0') {
        *destination = *source;
        source++;
        destination++;
    }

    *destination = *source;

    return _destination;
}

/*
 * This copies no more than n chars into the destination string from the source
 * string this will copy the null terminating char if it's within n chars.
 */
char* strncpy(char* destination, const char* source, size_t n)
{
    char* _destination = destination;

    while (n > 0) {
        *destination = *source;
        if (*source == '\0')
            break;
        source++;
        destination++;
        n--;
    }

    return _destination;
}

/*
 * This adds the source string onto the end of the destination string overiding
 * the null terminating '\0' char in the destination and copying the null
 * terminating '\0' from the source string. This returns a ptr to the start of
 * the destination string. This doesn't do any bounds checks.
 */
char* strcat(char* destination, const char* source)
{
    char* _destination = destination;

    while (*destination != '\0')
        destination++;

    strcpy(destination, source);
    return _destination;
}

/*
 * This compares len bytes of the two inputted ptrs. Returns zero if the
 * ptr's values are equal, a negative value if the value of the first ptr is
 * less than the second's value, and a positive value if the value of the first
 * ptr is greater than the second.
 */
i32 memcmp(const void* ptr0, const void* ptr1, size_t len)
{
    const u8* _ptr0 = ptr0;
    const u8* _ptr1 = ptr1;

    while (len != 0) {
        if (*_ptr0 != *_ptr1)
            return *_ptr0 - *_ptr1;
        _ptr0++;
        _ptr1++;
    }

    return 0;
}

/*
 * This compares the two inputted strings. Returns zero if the strings are
 * equal, a negative value if the first string is less than the second, and a
 * positive value if the first string is greater than the second.
 */
i32 strcmp(const char* string0, const char* string1)
{
    while (*string0 != '\0' || *string1 != '\0') {
        if (*string0 != *string1)
            return *string0 - *string1;
        string0++;
        string1++;
    }

    return 0;
}

/*
 * This compares no more than n chars of the two inputted strings. Returns zero
 * if the strings are equal, a negative value if the first string is less than
 * the second, and a positive value if the first string is greater than the
 * second.
 */
i32 strncmp(const char* string0, const char* string1, size_t n)
{
    while (*string0 != '\0' && n > 0) {
        if (*string0 != *string1)
            return *string0 - *string1;
        string0++;
        string1++;
        n--;
    }

    return 0;
}

/*
 * This returns the first occurrence of the inputted c as a i8 in the first n
 * bytes of the inputted source. This returns a ptr to the found index of the
 * inputted byte or NULLPTR if it isn't found.
 */
void* memchr(const void* source, i32 c, size_t n)
{
    const u8* _source = source;
    i8 _c = (i8)c;

    while (n > 0) {
        if (_c == *_source)
            return (void*)_source;
        n--;
    }

    return NULLPTR;
}

/*
 * This finds the first occurrence of the char of c in the inputted source
 * string and returns a ptr to it or NULLPTR if it isn't found.
 */
char* strchr(const char* source, int c)
{
    char _c = (char)c;

    while (*source != '\0') {
        if (*source == _c)
            return (char*)source;
        source++;
    }

    return _c == '\0' ? (char*)source : NULLPTR;
}

/*
 * This computes the maximum length of the inital segment of the first string
 * that doesn't contain any chars the second string contains. This doesn't
 * include the null terminating '\0' char.
 */
size_t strcspn(const char* string0, const char* string1)
{
    u8 char_table[256 >> 3];
    size_t len = 0;

    memset(char_table, 0, 256 >> 3);

    while (*string1 != '\0') {
        char_table[(*string1) >> 3] |= 1 << ((*string1) & 0x7);
        string1++;
    }

    while (*string0 != '\0') {
        if (char_table[(*string0) >> 3] & (1 << ((*string0) & 0x7)))
            return len;
        string0++;
        len++;
    }

    return len;
}

/*
 * This locates the first occurrence in the first inputted string of any char
 * in the second string and returns it or a NULLPTR if it isn't found. 
 */
char* strpbrk(const char* string0, const char* string1)
{
    u8 char_table[256 >> 3];

    memset(char_table, 0, 256 >> 3);

    while (*string1 != '\0') {
        char_table[(*string1) >> 3] |= 1 << ((*string1) & 0x7);
        string1++;
    }

    while (*string0 != '\0') {
        if (char_table[(*string0) >> 3] & (1 << ((*string0) & 0x7)))
            return (void*)string0;
        string0++;
    }

    return NULLPTR;
}

/*
 * This returns the last occurrence of the inputted c as a char in the inputted
 * string the null terminating '\0' char is considered part of the string. If
 * the inputted c as a char isn't found NULLPTR is returned.
 */
char* strrchr(const char* string, int c)
{
    char _c = (char)c;
    char* last_seen = NULLPTR;

    while (*string != '\0') {
        if (_c == *string)
            last_seen = (char*)string;
        string++;
    }

    return _c == *string ? (char*)string : last_seen;
}

/*
 * This returns the length of the maximum inital segment of the first inputted
 * string that consists entirely of chars from the second inputted string.
 */
size_t strspn(const char* string0, const char* string1)
{
    u8 char_table[256 >> 3];
    size_t len = 0;

    memset(char_table, 0, 256 >> 3);

    while (*string1 != '\0') {
        char_table[(*string1) >> 3] |= 1 << ((*string1) & 0x7);
        string1++;
    }

    while (*string0 != '\0') {
        if (!(char_table[(*string0) >> 3] & (1 << ((*string0) & 0x7))))
            return len;
        len++;
        string0++;
    }

    return len;
}

/*
 * This locates the first occurrence of the second inputted string in the first
 * inputted string excluding the null terminating '\0' char. This will return a
 * ptr to the first char of the found segment in the first inputted string.
 */
char* strstr(const char* string0, const char* string1)
{
    while (*string0 != '\0') {
        if (!strcmp(string0, string1))
            return (char*)string0;
        string0++;
    }
}

/* This sets n bytes of destination to c converted to a u8. */
void* memset(void* destination, int c, size_t n)
{
    u8* _destination = destination;
    char _c = (u8)c;

    while (n > 0) {
        *_destination = _c;
        _destination++;
        n--;
    }

    return destination;
}

/*
 * This counts and returns the number of chars in the inputted string. The
 * length of the string doesn't include the null termination '\0' char.
 */
size_t strlen(const char* string)
{
    size_t len = 0;

    while (*string != '\0') {
        len++;
        string++;
    }

    return len;
}
