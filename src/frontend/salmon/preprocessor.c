/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/salmon/preprocessor.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>

/* These are the characters considered white space while tokenizing. */
static const char white_space_chars[] = { ' ', '\t', '\n', '\0' };

/* These are the characters considered special when tokenizing. */
static const char special_chars[] = { '*', '/', '{', '}', ';', '^', '!', '&',
'|', ',', '(', ')', '$', '%', '@', '#', '\\', '.', '\"', '\'', '~', '.', '\0' };

/*
 * This takes in a file name and returns the tokenized file and the tokenized
 * file with '\n's for error handling.
 */
tokenize_file_return salmon_preprocess_file(char* file_name)
{
    tokenize_file_return _return = tokenize_file(file_name, \
    white_space_chars, special_chars);

    _return.special_chars = special_chars;
    _return.white_space_chars = white_space_chars;

    /* This is the output file. */
    vector new_file = vector_init(sizeof(char*), 4);

    START_PROFILING("preprocess file", "compile file");

    for (u32 i=0; i < VECTOR_SIZE(_return.token_vector); i++) {
        if (VECTOR_AT(&_return.token_vector, i, char*) == NULLPTR)
            continue;

        replace_C_const_chars(&_return.token_vector, i, special_chars);

        if (IS_VEC_END(_return.token_vector, i))
            break;

        replace_C_escape_codes(&_return.token_vector, &i, special_chars);

        // TODO: This /* */ code /* */ compiles the second comment because
        // skipping comments isn't done at the end of comments.
        skip_C_comments(&_return.token_vector, &i, special_chars, \
        white_space_chars);

        if (IS_VEC_END(_return.token_vector, i))
            break;

        vector_append(&new_file, vector_at(&_return.token_vector, i, false));
    }

    // for (u32 i=0; i < VECTOR_SIZE(new_file); i++)
        // printf("%s\n", *(char**)vector_at(&new_file, i, false));

    free(_return.token_vector.contents);
    _return.source_vector = new_file;

    END_PROFILING("preprocess file", true);

    return _return;
}