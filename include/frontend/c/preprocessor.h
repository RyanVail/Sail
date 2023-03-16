/*
 * This is the preprocessor for the C programming language.
 */

#ifndef FRONTEND_c_PREPROCESSOR_H
#define FRONTEND_c_PREPROCESSOR_H

#include<common.h>

/* This takes in a file name and returns a preprocessed version of it. */
vector C_preprocess_file(char* file_name);

#endif
