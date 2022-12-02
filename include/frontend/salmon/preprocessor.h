/*
 * This is the preprocessor for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_PREPROCESSOR_H
#define FRONTEND_SALMON_PREPROCESSOR_H

#include<common.h>

/* 
 * This takes in a file name and returns the preprocessed version of it.
 */
vector salmon_preprocess_file(char* file_name);

#endif