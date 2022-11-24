#include<common.h>
#include<frontend/common/tokenizer.h>

static char* white_space_chars;
static char* special_chars;

/*
 * This function goes through the tokenized file and skips NULL pointers till it
 * finds a valid token.
 */
void find_next_valid_token(vector *file, u32* current_index)
{
    for (; *current_index <= VECTOR_SIZE((*file))-1; *current_index += 1) {
        if (vector_at(file, *current_index, false) != NULL)
            return;
    }
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
    if (white_space_chars == NULL || special_chars == NULL) {
        printf("\"white_space_chars\" or \"special_chars\" were never set.\n");
        exit(-1);
    }
    #endif

    FILE* file_handle = fopen(file_name, "r");
    if (file_handle == NULL)
        handle_error(2);

    vector file_vector = vector_init_with(sizeof(char*), 8);
    // vector file_vector = { NULL, 0, 0, sizeof(char*) };

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

// TODO: If this function isn't needed remove it.
// /*
//  * This takes in the vector of a tokenized file and removes NULL pointers.
//  */
// void remove_null_pointers_from_tokenized_file(vector* _vector)
// {
//     u32 new_index = 0;
//     for (u32 real_index=0; real_index < VECTOR_SIZE((*_vector)); real_index++) {
//         if (vector_at(_vector, real_index, false) == NULL)
//             continue;
//         if (*(char**)vector_at(_vector, real_index, false) != NULL) {
//             *(char**)vector_at(_vector, new_index, false) = \
//                 *(char**)vector_at(_vector, real_index, false);
//             new_index++;
//         }
//     }
//     _vector->apparent_size = new_index;
//     vector_try_to_shrink(_vector);
// }


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
