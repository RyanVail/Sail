#include<common.h>
#include<frontend/common/tokenizer.h>

static char* white_space_chars;
static char* special_chars;

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
 * This returns truw if the inputed char is a special char
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
    if (white_space_chars == NULL || special_chars == NULL) {
        printf("\"white_space_chars\" or \"special_chars\" were never set.\n");
        exit(-1);
    }
    #endif

    FILE* file_handle = fopen(file_name, "r");
    if (file_handle == NULL)
        handle_error(2);

    vector file_vector = { NULL, 0, 0, sizeof(char*) };

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
            char* _tmp = malloc(sizeof(char) * buffer_index);
            if (_tmp == NULL)
                    handle_error(0);
            memcpy(_tmp, token_buffer, buffer_index);
            vector_append(&file_vector, &_tmp);

            tokenizer_save_special_char_label:
            if (special_char) {
                char* _tmp = malloc(sizeof(char) * 2);
                if (_tmp == NULL)
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
        char** _t = vector_pop(_vector);
        if (*_t != NULL)
            free(*_t);
        free(_t);
    }
    free(_vector->contents);   
}
