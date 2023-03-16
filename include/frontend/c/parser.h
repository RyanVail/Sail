#ifndef FRONTEND_C_PARSER_H
#define FRONTEND_C_PARSER_H

#include<common.h>
#include<evaluate.h>

/*
 * This function reads in the C source file referenced by "file_name" processes
 * and turns it into intermediates.
 */
void C_file_into_intermediates(char* file_name);

/*
 * This function takes in the starting index of an operation and passes the
 * "operator" type of every operator into the inputted "operator_func".
 * The parsed operators are passed into the inputted "operand_func". After
 * reaching "ending_token" this will stop parsing the equation if "ending_token"
 * is a NULLPTR nothing is done.
 */
bool C_parse_operation(char*** token, void (*operator_func)(operator), \
char** (*operand_func)(char**), char** ending_token);

#endif