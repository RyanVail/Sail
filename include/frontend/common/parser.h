/*
 * This contains a lot of commonly used parsing function branching a front end
 * to the intermediate stage.
 */

#ifndef FRONTEND_COMMON_PARSER_H
#define FRONTEND_COMMON_PARSER_H

#include<common.h>
#include<types.h>

/*
 * This parses and returns the type of the same name as the inputed string. This
 * assumes that the string is in a array and that there are no NULL pointers in
 * the array. This also assumes that the pointer char is a special char. If we
 * didn't get a type the returning type kind will be equal to 255.
 */
type parse_type(char** string_ptr);

/*
 * If the inputed name is invalid it will return true. This is cap sensitive.
 * Along with the current INVALID_NAMES being invalid, any special tokens, and
 * types are counted as invalid. If the name starts with a number it is also
 * considered invalid.
 */
bool is_invalid_name(char* name);

#endif