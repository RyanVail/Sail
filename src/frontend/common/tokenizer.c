#include<common.h>
#include<frontend/common/tokenizer.h>

/*
 * This function goes through the tokenized file and skips NULLPTRs till it
 * reaches a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index)
{
    *current_index += 1;
    for (; *current_index < VECTOR_SIZE(*file)-1; *current_index += 1)
        if (VECTOR_AT(file, *current_index, char*) != NULLPTR)
            return;
}

/*
 * This takes in a file name and returns the tokenized version of the file. If
 * the DESCRIPTIVE_ERRORS flag is set to true this will also return the lines of
 * source code.
 */
tokenize_file_return tokenize_file(const char* file_name,
const char* white_space_chars, const char* special_chars)
{
    #if DEBUG
    if (white_space_chars == NULLPTR || special_chars == NULLPTR) {
        printf("\"white_space_chars\" or \"special_chars\" were never set.\n");
        exit(-1);
    }
    #endif

    START_PROFILING("tokenize file", "compile file");

    i32 file_descriptor = fsopen(file_name, "r");
    if (file_descriptor < 0)
        HANDLE_COMMON_ERROR(2);

    #if DESCRIPTIVE_ERRORS
    vector source_vector = vector_init(sizeof(char*), 8);
    char source_line_buffer[TOKENIZER_SOURCE_LINE_BUFFER_SIZE];
    u32 source_line_index = 0;
    #endif

    token current_token;

    vector file_vector = vector_init(sizeof(token), 8);

    char file_buffer[TOKENIZER_FILE_BUFFER_SIZE];

    // TODO: Use a init value that is based in reality.
    vector this_token = vector_init(sizeof(char), 6);

    u32 _read = 1;

    char special_char = '\0';

    while (_read) {
        _read = read(file_descriptor, file_buffer, TOKENIZER_FILE_BUFFER_SIZE);

        special_char = '\0';
        u32 i = 0;
        for (; i < _read; i++) {
            #if DESCRIPTIVE_ERRORS
            if (file_buffer[i] == '\n') {
                char* new_source = malloc(source_line_index+1);
                new_source[source_line_index] = '\0';
                CHECK_MALLOC(new_source);
                vector_append(&source_vector, &new_source);
                source_line_index = 0;
            }
            source_line_buffer[source_line_index] = file_buffer[i];
            source_line_index++;
            #endif

            if (is_special_char(file_buffer[i], special_chars)) {
                special_char = file_buffer[i];
            } else if (!is_white_space_char(file_buffer[i], white_space_chars)) {
                vector_append(&this_token, &file_buffer[i]);
                continue;
            }

            if (VECTOR_SIZE(this_token) == 0) {
                if (special_char)
                    goto tokenizer_save_special_char_label;
                continue;
            }

            char* _tmp = malloc(VECTOR_SIZE(this_token) + 1);
            CHECK_MALLOC(_tmp);
            memcpy(_tmp, this_token.contents, VECTOR_SIZE(this_token));
            _tmp[VECTOR_SIZE(this_token)] = '\0';

            /* The source line is the current line that hasn't been added. */
            #if DESCRIPTIVE_ERRORS
            current_token.source = vector_at (
                &file_vector,
                VECTOR_SIZE(file_vector),
                true
            );

            current_token.line_index = source_line_index;
            #endif

            current_token.content = _tmp;

            vector_append(&file_vector, &current_token);

            tokenizer_save_special_char_label:
            if (special_char) {
                char* _tmp = malloc(2);
                CHECK_MALLOC(_tmp);
                _tmp[0] = special_char;
                _tmp[1] = '\0';

                /* The source line is the current line that hasn't been added. */
                #if DESCRIPTIVE_ERRORS
                current_token.source = vector_at (
                    &file_vector,
                    VECTOR_SIZE(file_vector),
                    true
                );

                current_token.line_index = source_line_index;
                #endif

                current_token.content = _tmp;

                vector_append(&file_vector, &current_token);
                special_char = '\0';
            }

            this_token.apparent_size = 0;
        }
    }
    close(file_descriptor);

    END_PROFILING("tokenize file", true);

    tokenize_file_return returning;
    returning.token_vector = file_vector;

    #if DESCRIPTIVE_ERRORS
    returning.source_vector = source_vector;
    #endif

    return returning;
}

/* This takes in the vector of a tokenized file and frees it. */
void free_tokenized_file_vector(vector* _vector)
{
    if (_vector->size == 0)
        return;

    // TODO: Benchmark this against a for loop through every item
    while (VECTOR_SIZE(*_vector)) {
        char** token = vector_pop(_vector);
        if (*token != NULLPTR)
            free(*token);
        free(token);
    }
    free(_vector->contents);   
}
