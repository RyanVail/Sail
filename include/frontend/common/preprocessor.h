/*
 * This contains commonly used preprocessor functions.
 */

#ifndef FRONTEND_COMMON_PREPROCESSOR_H
#define FRONTEND_COMMON_PREPROCESSOR_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function is meant to be run during the preprocessor loop and it skips
 * C style comments. This can leave the "current_index" pointing to the end of
 * the file, that has to be accounted for.
 */
void skip_C_comments(vector* file, u32* current_index);

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * constant char strings with their constant values. This replaces other tokens
 * inside of the string with NULL pointers.
 */
void replace_C_const_chars(vector* file, u32 current_index);

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * escape codes with their true values. This will replace the other tokens
 * inside of the backslash with null pointers. This doesn't check for NULL
 * pointers so it must be done before this is called.
 */
void replace_C_escape_codes(vector* file, u32* current_index);

#endif