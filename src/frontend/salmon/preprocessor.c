/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/salmon/preprocessor.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>

static char white_space_salmon[] = { ' ', '\t', '\n', '\0' };
static char special_salmon[] = { '*', '/', '{', '}', ';', '^', '!', '&', '|', \
',', '(', ')', '$', '%', '@', '#', '\\', '.', '\"', '\'', '~', '.', '\0' };

/*
 * This takes in a file name and returns the preprocessed version of it.
 */
vector salmon_preprocess_file(char* file_name)
{
    set_tokenizer_chars(white_space_salmon, special_salmon);
    vector tokenized_file = tokenize_file(file_name).token_vector;
    vector new_file = { NULLPTR, 0, 0, .type_size = sizeof(char*) };

    START_PROFILING("preprocess file", "compile file");

    for (u32 i=0; i < tokenized_file.apparent_size; i++) {
        if (*(char**)vector_at(&tokenized_file, i, false) == NULLPTR)
            continue;

        replace_C_const_chars(&tokenized_file, i);

        if (IS_VEC_END(tokenized_file, i))
            break;

        replace_C_escape_codes(&tokenized_file, &i);

        // TODO: This /* */ code /* */ compiles the second comment because
        // skipping comments isn't done at the end of comments.
        skip_C_comments(&tokenized_file, &i);
        if (IS_VEC_END(tokenized_file, i))
            break;

        // printf("%s\n", *(char**)vector_at(&tokenized_file, i, false));
        vector_append(&new_file, vector_at(&tokenized_file, i, false));
    }

    // for (u32 i=0; i < VECTOR_SIZE(new_file); i++)
        // printf("%s\n", *(char**)vector_at(&new_file, i, false));

    free(tokenized_file.contents);

    END_PROFILING("preprocess file", true);

    return new_file;
}