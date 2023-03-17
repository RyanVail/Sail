/*
 * This is the preprocessor for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_PREPROCESSOR_H
#define FRONTEND_SALMON_PREPROCESSOR_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This takes in a file name and returns the tokenized file and the tokenized
 * file with '\n's for error handling.
 */
tokenize_file_return salmon_preprocess_file(char* file_name);

#endif