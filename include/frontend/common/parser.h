/*
 * This contains a lot of commonly used parsing function branching a front end
 * to the intermediate stage.
 */

#ifndef FRONTEND_COMMON_PARSER_H
#define FRONTEND_COMMON_PARSER_H

#include<common.h>
#include<types.h>

/*
 * This parses and returns the given type modifiers. This will incrament token
 * till it reaches the end of the modifiers. unsigned and signed modifiers will
 * change the first bit of the returinging kind. This will skip tokens that are
 * equal to NULLPTR and returns when it doesn't hit a modifier.
 */
type_kind get_type_modifier(char*** token);

/*
 * This parses and returns the type of the same name as the inputed string. This
 * assumes that the string is in a array and that there are no NULL pointers in
 * the array. This also assumes that the pointer char is a special char. If we
 * didn't get a type the returning type kind will be equal to 255.
 */
type get_type(char** token);

typedef struct is_ascii_float_return is_ascii_float_return;
/*
 * This goes through the inputed string and returns 0 if it isn't a float, 1 if
 * it is a float, and 2 if it's a double indicated by the trailing 'd' or 'f',
 * but defaulting to a float.
 */
is_ascii_float_return is_ascii_float(char** starting_float_token);

/*
 * This returns the f64 representation of the inputed token.
 */
f64 get_ascii_float(char** starting_float_token, char** ending_float_token);

/*
 * This returns the numeral value of an ASCII string.
 */
i64 get_ascii_number(char* num_string);

/*
 * This goes through a string and returns true if it is an ASCII number.
 */
bool is_ascii_number(char* num_string);

/*
 * This goes from the current position in the file till it reaches a '\n' and
 * returns the file index of the '\n'.
 */
u32 get_end_of_line(vector* file, u32 i);

/*
 * If the inputed name is invalid it will return true. This is cap sensitive.
 * Along with the current INVALID_NAMES being invalid, any special tokens, and
 * types are counted as invalid. If the name starts with a number it is also
 * considered invalid.
 */
bool is_invalid_name(char* name);

/* This represents the possible return types from "is_ascii_float". */
typedef enum is_ascii_float_return_float_type {
    FLOAT_RETURN_NOT_A_FLOAT,
    FLOAT_RETURN_SINGLE_FLOAT,
    FLOAT_RETURN_DOUBLE_FLOAT,
} is_ascii_float_return_float_type;

/* struct ascii_float_return - This struct is only used as the return from the
 * function "is_ascii_float"
 * @end_ptr: This is a pointer to the ending token of the ASCII float
 * @token_length: This is the number of tokens this float encompasses
 * @float_type: This is the type of the float
 */
typedef struct is_ascii_float_return {
    char** end_ptr;
    u32 token_length;
    is_ascii_float_return_float_type type;
} is_ascii_float_return;

#endif