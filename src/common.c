#include<common.h>

/*
 * This takes in a string an convertes the english characters to lower case.
 */
void strlower(char* _string)
{
    for (u32 i=0; i < strlen(_string); i++)
        if (_string[i] >= 65 && _string[i] <= 90)
            _string[i] |= 32;
}