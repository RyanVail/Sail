#ifndef TOKENIZER_H
#define TOKENIZER_H

#include<common.h>

#if DESCRIPTIVE_ERRORS
/* struct tokenize_file_return - This is the return of the "toknize_file"
 * function
 * @token_vector: This is the vector of tokens.
 * @source_vector: This is the vector of source code lines
 */
typedef struct tokenize_file_return {
    vector token_vector;
    vector source_vector;
} tokenize_file_return;

/* struct token - This represents a single descriptive token
 * @content: This is the contents of this token
 * @source: This is a ptr to the line of source code this token came from
 * @line_index: This is the index of the line of source this token came from
 * printing error relies on this number going up linearly.
 */
typedef struct token {
    char* content;
    char* source;
    u16 line_index;
} token;
#else
/* struct tokenize_file_return - This is the return of the "toknize_file"
 * function
 * @token_vector: This is the vector of tokens.
 */
typedef struct tokenize_file_return {
    vector token_vector;
} tokenize_file_return;

/* struct token - This represents a single token
 * @content: This is the contents of the token
 */
typedef struct token {
    char* content;
} token;
#endif

/*
 * This function goes through the tokenized file and skips NULL pointers till it
 * finds a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index);

/*
 * This takes in a file name and returns the tokenized version of the file. If
 * the DESCRIPTIVE_ERRORS flag is set to true this will also return the lines of
 * source code.
 */
tokenize_file_return tokenize_file(char* file_name);

/*
 * This function is called from the frontend being used and define the static
 * "white_space_chars" and "special_chars" with the given values.
 */
void set_tokenizer_chars(char* _white_space_chars, char* _special_chars);

/*
 * This takes in the vector of a tokenized file and frees it.
 */
void free_tokenized_file_vector(vector* _vector);

/*
 * This frees the tokenizer chars.
 */
void free_tokenizer_chars();

/*
 * This returns true if the inputted char is a white space character.
 */
bool is_white_space_char(char _char);

/*
 * This returns true if the inputted char is a special char.
 */
bool is_special_char(char _char);

#endif
