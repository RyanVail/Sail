#include<common.h>
#include<frontend/common/tokenizer.h>

static char* white_space_chars = NULLPTR;
static char* special_chars = NULLPTR;

/*
 * This function goes through the tokenized file and skips NULLPTRs till it
 * reaches a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index)
{
    *current_index += 1;
    for (; *current_index < VECTOR_SIZE((*file))-1; *current_index += 1)
        if (*(char**)vector_at(file, *current_index, false) != NULLPTR)
            return;
}

/*
 * This returns true if the inputted char is a white space character.
 */
bool is_white_space_char(char _char)
{
    for (u32 i=0; white_space_chars[i] != '\0'; i++)
        if (_char == white_space_chars[i])
            return true;
    return false;
}

/*
 * This returns true if the inputted char is a special char.
 */
bool is_special_char(char _char)
{
    for (u32 i=0; special_chars[i] != '\0'; i++)
        if (_char == special_chars[i])
            return true;
    return false;
}

/*
 * This takes in a file name and returns the tokenized version of the file. If
 * the DESCRIPTIVE_ERRORS flag is set to true this will also return the lines of
 * source code.
 */
tokenize_file_return tokenize_file(char* file_name)
{
    #if DEBUG
    if (white_space_chars == NULLPTR || special_chars == NULLPTR) {
        printf("\"white_space_chars\" or \"special_chars\" were never set.\n");
        exit(-1);
    }
    #endif

    START_PROFILING("tokenize file", "compile file");

    FILE* file_handle = fopen(file_name, "r");
    if (file_handle == NULLPTR)
        HANDLE_COMMON_ERROR(2);

    #if DESCRIPTIVE_ERRORS
    vector source_vector = vector_init(sizeof(char*), 8);
    char source_line_buffer[TOKENIZER_SOURCE_LINE_BUFFER_SIZE];
    u32 source_line_index = 0;
    token current_token;
    #endif

    vector file_vector = vector_init(sizeof(token), 8);

    char file_buffer[TOKENIZER_FILE_BUFFER_SIZE];

    // TODO: This has to be rewritten so buffer overflow isn't a possibility.
    char token_buffer[TOKENIZER_TOKEN_BUFFER_SIZE];

    u32 _read = 1;

    char special_char = '\0';

    while (_read) {
        _read = fread(file_buffer, sizeof(char), TOKENIZER_FILE_BUFFER_SIZE, \
        file_handle);

        u32 buffer_index = 0;
        special_char = '\0';
        u32 i = 0;
        for (; i < _read; i++) {
            if (file_buffer[i] == '\n') {
                char* new_source = malloc(source_line_index+1);
                new_source[source_line_index] = '\0';
                CHECK_MALLOC(new_source);
                vector_append(&source_vector, &new_source);
                source_line_index = 0;
            }

            source_line_buffer[source_line_index] = file_buffer[i];
            source_line_index++;

            if (is_special_char(file_buffer[i])) {
                special_char = file_buffer[i];
            } else if (!is_white_space_char(file_buffer[i])) {
                token_buffer[buffer_index] = file_buffer[i];
                buffer_index++;
                continue;
            }

            if (buffer_index == 0) {
                if (special_char)
                    goto tokenizer_save_special_char_label;
                continue;
            }

            token_buffer[buffer_index] = '\0';
            buffer_index++;
            char* _tmp = malloc(buffer_index);
            CHECK_MALLOC(_tmp);
            memcpy(_tmp, token_buffer, buffer_index);

            /* The source line is the current line that hasn't been added. */
            current_token.source = vector_at(&file_vector, \
                VECTOR_SIZE(file_vector), true);
            current_token.content = _tmp;
            current_token.line_index = source_line_index;

            vector_append(&file_vector, &_tmp);

            tokenizer_save_special_char_label:
            if (special_char) {
                char* _tmp = malloc(2);
                CHECK_MALLOC(_tmp);
                _tmp[0] = special_char;
                _tmp[1] = '\0';

                /* The source line is the current line that hasn't been added. */
                current_token.source = vector_at(&file_vector, \
                    VECTOR_SIZE(file_vector), true);
                current_token.content = _tmp;
                current_token.line_index = source_line_index;

                vector_append(&file_vector, &_tmp);
                special_char = '\0';
            }

            buffer_index = 0;
        }
    }
    fclose(file_handle);

    END_PROFILING("tokenize file", true);

    tokenize_file_return returning;
    returning.token_vector = file_vector;

    #if DESCRIPTIVE_ERRORS
    returning.source_vector = source_vector;
    #endif

    return returning;
}

/*
 * This function is called from the frontend being used and define the static
 * "white_space_chars" and "special_chars" with the given values.
 */
void set_tokenizer_chars(char* _white_space_chars, char* _special_chars)
{
    white_space_chars = _white_space_chars;
    special_chars = _special_chars;
}

/*
 * This takes in the vector of a tokenized file and frees it.
 */
void free_tokenized_file_vector(vector* _vector)
{
    if (_vector->size == 0)
        return;

    // TODO: Benchmark this against a for loop through every item
    while (VECTOR_SIZE((*_vector))) {
        char** token = vector_pop(_vector);
        if (*token != NULLPTR)
            free(*token);
        free(token);
    }
    free(_vector->contents);   
}
