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
    const char* special_chars;
    const char* white_space_chars;
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
 * This function goes through the tokenized file and skips NULLPTRs till it
 * reaches a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index);

// TODO: This should take in a bool array or bit array to check if the inputted
// char is a special char for faster tokenizing and a little less storage usage.
/*
 * This takes in a file name and returns the tokenized version of the file. If
 * the DESCRIPTIVE_ERRORS flag is set to true this will also return the lines of
 * source code.
 */
tokenize_file_return tokenize_file(const char* file_name, \
const char* white_space_chars, const char* special_chars);

/* This takes in the vector of a tokenized file and frees it. */
void free_tokenized_file_vector(vector* _vector);

/* This returns true if the inputted char is a special char. */
static inline bool is_special_char(char _char, const char* special_chars)
{
    return strchr(special_chars, _char) != NULLPTR;
}

/* This returns true if the inputted char is a white space char. */
static inline bool is_white_space_char(char _char, \
const char* white_space_chars)
{
    return strchr(white_space_chars, _char) != NULLPTR;
}

#endif
