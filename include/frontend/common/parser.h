/*
 * This contains a lot of commonly used parsing function branching a frontend
 * to the intermediate stage.
 */

#ifndef FRONTEND_COMMON_PARSER_H
#define FRONTEND_COMMON_PARSER_H

#include<common.h>
#include<types.h>

/* The types of printing errors can have between ranges. */
typedef enum error_range_type {
    ERROR_RANGE_TYPE_NORMAL,
    ERROR_RANGE_TYPE_SUCCESS,
    ERROR_RANGE_TYPE_FAILED,
    ERROR_RANGE_TYPE_CORRECTION,
    // TODO: Fully implement this.
    ERROR_RANGE_TYPE_ADDITION,
} error_range_type;

/* struct error_token_range - This is the range of tokens that are printed
 * during error reports
 * @type: The type of the error, used to decide which color to use
 * @spaced: If the tokens should be printed with spaced between them
 * @starting_token: The starting range of the error tokens
 * @ending_token: The ending range of the error tokens
 * @overide_token: If this isn't a NULLPTR this will be printed out instead of
 * the tokens between the starting and ending token and without the type color
 * ending token should still be set to the last token so the index of the token
 * can still be found. This will get freeded after printing // TODO: This dumb
 * index finding thing should be replaced with these ranges also holding their
 * indexes.
 */
typedef struct error_token_range {
    error_range_type type;
    bool spaced;
    char** starting_token;
    char** ending_token;
    char* overide_token;
} error_token_range;

/*
 * This handles common errors that come up and shouldn't be set to the main
 * error handler, rather be called on specific error types.
 */
error_token_range parser_handle_error(intermediate_pass* _pass, \
parsing_error _error, char** token);

#if DESCRIPTIVE_ERRORS
/*
 * This function and enum is used by "parser_handle_error" to give descriptive
 * errors on invalid names.
 */
typedef enum invalid_name_type {
    INVALID_NAME_TYPE_INVALID_CHAR = 1,
    INVALID_NAME_TYPE_STARTS_WITH_NUMBER = 2,
    INVALID_NAME_TYPE_IS_IN_INVALID_NAMES = 4,
    INVALID_NAME_TYPE_IS_A_TYPE = 8,
} invalid_name_type;
/*
 * This function and enum is used by "parser_handle_error" to give descriptive
 * errors on invalid names.
 */
invalid_name_type get_why_invalid_name(intermediate_pass* _pass, char* name);
#endif

/*
 * This parses and returns the type of the same name as the inputted string.
 * This will also read and set the ptr count of the returned type. This assumes
 * that the string is in a array and that there are no NULL pointers in the
 * array. This also assumes that the pointer char is a special char. If there's
 * no type a type the returned type's kind will be NO_TYPE. This sets errno on
 * errors. This also sets errno_value to the "inital_token" appon returning
 * NO_TYPE and if type ptrs are unequal.
 */
type get_type(intermediate_pass* _pass, vector* file, u32 index);

typedef struct is_ascii_float_return is_ascii_float_return;
/*
 * This goes through the inputted string and returns 0 if it isn't a float, 1 if
 * it is a float, and 2 if it's a double indicated by the trailing 'd' or 'f',
 * but defaulting to a float.
 */
is_ascii_float_return is_ascii_float(char** starting_float_token);

/*
 * This returns the f64 representation of the inputted token.
 */
f64 get_ascii_float(char** starting_float_token, char** ending_float_token);

/*
 * This returns the numeral value of an ASCII string. This supports negative
 * signs but doesn't support suffix or prefixes.
 */
num get_ascii_number(char* num_string);

/*
 * Returns true if the inputted number string contains all number characters
 * this supports negative numbers and suffixes, but doesn't support numbers with
 * decimal points. Prefixes and suffixes both need to be one character and
 * prefixes are assumed to start with a '0' so that has to be excluded from the
 * inputted "prefixes" string of chars.
 */
bool is_ascii_number(char* num_string, const char* prefixes,
const char* suffixes);

/*
 * This goes from the current position in the file till it reaches a '\n' and
 * returns the file index of the '\n'.
 */
u32 get_end_of_line(vector* file, u32 i);

/*
 * If the inputted name is invalid it will return true. This is cap sensitive.
 * Any names found in the inputted intermediate pass' front end's invalid_names
 * and type_names are considered invalid. If the name starts with a number it is
 * also considered invalid.
 */
bool is_invalid_name(intermediate_pass* _pass, char* name);

/* This represents the possible return types from "is_ascii_float". */
typedef enum is_ascii_float_return_float_type {
    FLOAT_RETURN_NOT_A_FLOAT,
    FLOAT_RETURN_SINGLE_FLOAT,
    FLOAT_RETURN_DOUBLE_FLOAT,
} is_ascii_float_return_float_type;

// TODO: This should be named "float_parsing_data" or something and the main
// function should be named something like "try_get_float".
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