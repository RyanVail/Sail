/*
 * This contains commonly used preprocessor functions.
 */

#ifndef FRONTEND_COMMON_PREPROCESSOR_H
#define FRONTEND_COMMON_PREPROCESSOR_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function is meant to be run during the preprocessor loop and it skips
 * C style comments.
 */
void remove_C_comments(vector* file, u32* current_index);

#endif