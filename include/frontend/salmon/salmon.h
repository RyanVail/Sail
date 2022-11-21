/*
 * This is the front end for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_H
#define FRONTEND_SALMON_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns a vector of those intermediate tokens.
 */
vector salmon_file_into_intermediate(char* file_name);

#endif