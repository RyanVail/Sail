/*
 * This is the frontend for the Salmon programming language.
 */

#ifndef FRONTEND_SALMON_H
#define FRONTEND_SALMON_H

#include<common.h>
#include<frontend/common/tokenizer.h>
#include<intermediate/pass.h>

/*
 * This function is used to init the data in an intermediate pass to the salmon
 * front end for things like variable names and tokenizer characters. This is
 * used when initing salmon parsing and when salmon parsing data is needed on an
 * intermediate pass that doesn't contain it.
 */
void salmon_parser_init_front_end(intermediate_pass* _pass);

/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns an "intermediate_pass".
 */
intermediate_pass salmon_file_into_intermediates(char* file_name);

/*
 * This function acts as the error handling function while parsing Salmon source
 * this reads errno and handles errors based on the value.
 */
void salmon_parser_handle_error();

#endif