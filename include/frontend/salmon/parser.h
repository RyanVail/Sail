/*
 * This is the frontend for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_H
#define FRONTEND_SALMON_H

#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function turns a single salmon source file into intermediate tokens.
 * The tokens are stored in "frontend/common/tokenizer.c"
 */
void salmon_file_into_intermediates(char* file_name);

/*
 * This function acts as the error handling function while parsing Salmon source
 * this reads errno and handles errors based on the value.
 */
void salmon_parser_handle_error();

#endif