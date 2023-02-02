/*
 * This is the front end for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_H
#define FRONTEND_SALMON_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function turns a single salmon source file into intermediate tokens.
 * The tokens are stored in "frontend/common/tokenizer.c"
 */
void salmon_file_into_intermediate(char* file_name);

#endif