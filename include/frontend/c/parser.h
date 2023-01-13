#ifndef FRONTEND_C_PARSER_H
#define FRONTEND_C_PARSER_H

#include<common.h>

/*
 * This function reads in the C source file refrenced by "file_name" processes
 * and turns it into intermediates.
 */
void C_file_into_intermediate(char* file_name);

#endif