#include<common.h>
#include<frontend/common/tokenizer.h>
#if DEBUG && linux
#include<time.h>
#include<cli.h>
#endif

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
 * This returns true if the inputed char is a white space character
 */
bool is_white_space_char(char _char)
{
    for (u32 i=0; white_space_chars[i] != '\0'; i++)
        if (_char == white_space_chars[i])
            return true;
    return false;
}

/*
 * This returns true if the inputed char is a special char
 */
bool is_special_char(char _char)
{
    for (u32 i=0; special_chars[i] != '\0'; i++)
        if (_char == special_chars[i])
            return true;
    return false;
}

/*
 * This takes in a file name and returns the tokenized version of the file.
 */
vector tokenize_file(char* file_name)
{
    #if DEBUG
    if (white_space_chars == NULLPTR || special_chars == NULLPTR) {
        printf("\"white_space_chars\" or \"special_chars\" were never set.\n");
        exit(-1);
    }
    #endif
    #if DEBUG && linux
    clock_t starting_time = clock();
    #endif

    FILE* file_handle = fopen(file_name, "r");
    if (file_handle == NULLPTR)
        handle_error(2);

    vector file_vector = vector_init_with(sizeof(char*), 8);

    char file_buffer[FILE_BUFFER_SIZE];
    char token_buffer[512];

    u32 _read = 1;

    char special_char = '\0';

    while (_read) {
        _read = fread(file_buffer, sizeof(char), FILE_BUFFER_SIZE, file_handle);
        u32 buffer_index = 0;
        special_char = '\0';
        u32 i = 0;
        for (; i < _read; i++) {
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
            if (_tmp == NULLPTR)
                handle_error(0);
            memcpy(_tmp, token_buffer, buffer_index);
            vector_append(&file_vector, &_tmp);

            tokenizer_save_special_char_label:
            if (special_char) {
                char* _tmp = malloc(2);
                if (_tmp == NULLPTR)
                    handle_error(0);
                _tmp[0] = special_char;
                _tmp[1] = '\0';
                vector_append(&file_vector, &_tmp);
                special_char = '\0';
            }

            buffer_index = 0;
        }
    }
    fclose(file_handle);

    #if DEBUG && linux
    if (get_global_cli_options()->time_compilation)
        printf("Took %f ms to tokenize file.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f);
    #endif

    return file_vector;
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
