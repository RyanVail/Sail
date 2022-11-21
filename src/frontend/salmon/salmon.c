/*
 * This is the front end for the Salmon programming language.
 */

#include<frontend/salmon/salmon.h>

char white_space[] = { ' ', '\t', '\n' };
char special[] = { '*', '/', '{', '}', ';', '^', '!', '&', '|', ',', '(', \
')', '$', '%', '@', '#', '\\', '.', '\"', '\'' };

/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns a vector of those intermediate tokens.
 */
vector salmon_file_into_intermediate(char* file_name)
{
    set_tokenizer_chars(white_space, special);
    vector tokenized_file = tokenize_file(file_name);

    for (u32 i=0; i < tokenized_file.apparent_size; i++) {
        if (*(char**)vector_at(&tokenized_file, i, false)) {
            if (**(char**)vector_at(&tokenized_file, i, false) == '\\')
                replace_C_escape_codes(&tokenized_file, &i);
            else 
                remove_C_comments(&tokenized_file, &i);
            printf("%s\n", *(char**)vector_at(&tokenized_file, i, false));
        }
    }

    return tokenized_file;
}