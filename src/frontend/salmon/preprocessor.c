/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/salmon/preprocessor.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>

char white_space[] = { ' ', '\t', '\n' };
char special[] = { '*', '/', '{', '}', ';', '^', '!', '&', '|', ',', '(', \
')', '$', '%', '@', '#', '\\', '.', '\"', '\'' };

/* 
 * This takes in a file name and returns the preprocessed version of it.
 */
inline vector salmon_preprocess_file(char* file_name)
{
    set_tokenizer_chars(white_space, special);
    vector tokenized_file = tokenize_file(file_name);
    vector new_file = { NULL, 0, 0, sizeof(char*) };

    for (u32 i=0; i < tokenized_file.apparent_size; i++) {
        if (*(char**)vector_at(&tokenized_file, i, false) == NULL)
            continue;

        replace_C_const_chars(&tokenized_file, i);
        replace_C_escape_codes(&tokenized_file, &i);
        skip_C_comments(&tokenized_file, &i);
        if (i == VECTOR_SIZE(tokenized_file))
            break;

        vector_append(&new_file, vector_at(&tokenized_file, i, false));
    }

    // for (u32 i=0; i < VECTOR_SIZE(new_file); i++)
        // printf("%s\n", *(char**)vector_at(&new_file, i, false));

    free(tokenized_file.contents);
    return new_file;
}
