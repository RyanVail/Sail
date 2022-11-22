#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<common.h>

/*
 * This function goes through the tokenized file and skips NULL pointers till it
 * finds a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index);

/*
 * This takes in a file name and returns the tokenized version of the file.
 */
vector tokenize_file(char* file_name);

/*
 * This function is called from the frontend being used and define the static
 * "white_space_chars" and "special_chars" with the given values.
 */
void set_tokenizer_chars(char* _white_space_chars, char* _special_chars);

// TODO: If this function isn't needed remove it.
// /*
//  * This takes in the vector of a tokenized file and removes NULL pointers.
//  */
// void remove_null_pointers_from_tokenized_file(vector* _vector);

/*
 * This takes in the vector of a tokenized file and frees it.
 */
void free_tokenized_file_vector(vector* _vector);

/*
 * This frees the tokenizer chars
 */
void free_tokenizer_chars();

/*
 * This returns true if the inputed char is a white space character
 */
bool is_white_space_char(char _char);

/*
 * This returns truw if the inputed char is a special char
 */
bool is_special_char(char _char);

#endif
