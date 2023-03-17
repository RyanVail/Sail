/*
 * This contains commonly used preprocessor functions.
 */

#include<frontend/common/preprocessor.h>

/*
 * This is a private helper function for "skip_C_comments". Return true if it
 * read a comment. This keeps getting calling till it doesn't read a comment to
 * make sure consecutive comments are not skipped.
 */
bool skip_C_comment(vector* file, u32* current_index)
{
    if (IS_VEC_END(*file, *current_index))
        return false;

    if (*(char**)vector_at(file, *current_index, false) == NULLPTR \
    || **(char**)vector_at(file, *current_index, false) != '/')
        return false;

    u32 tmp_index = *current_index;
    find_next_valid_token(file, &tmp_index);
    if (IS_VEC_END(*file, tmp_index))
        return false;

    if (**(char**)vector_at(file, tmp_index, false) != '*' \
    || IS_VEC_END(*file, tmp_index))
        return false;

    for (; *current_index <= VECTOR_SIZE(*file)-2;) {
        char first_token = **(char**)vector_at(file, *current_index, false);

        free(*(char**)vector_at(file, *current_index, false));
        *(char**)vector_at(file, *current_index, false) = NULL;

        find_next_valid_token(file, current_index);
        char second_token = **(char**)vector_at(file, *current_index, false);

        if (first_token == '*' && second_token == '/') {
            free(*(char**)vector_at(file, *current_index, false));
            *(char**)vector_at(file, *current_index, false) = NULL;
            find_next_valid_token(file, current_index);
            return true;
        }
    }
}

/*
 * This function is meant to be run during the preprocessor loop and it skips
 * C style comments. This returns true if it skipped a comment and in which case
 * needs to be run again. This can leave the "current_index" pointing to the end of
 * the file, that has to be accounted for.
 */
void skip_C_comments(vector* file, u32* current_index, \
const char* special_chars, const char* white_space_chars)
{
    #if DEBUG
    if (!is_special_char('/', special_chars) \
    && !is_special_char('*', special_chars))
        send_error( \
        "'/' and '*' should be a special chars for C style comment removal");
    if (is_white_space_char('/', white_space_chars))
        send_error("'/' can't be white space for C style comment removal");
    if (is_white_space_char('*', white_space_chars))
        send_error("'*' can't be white space for C style comment removal");
    #endif

    while (skip_C_comment(file, current_index));
}

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * constant char strings with their constant values. This replaces other tokens
 * inside of the string with NULL pointers.
 */
void replace_C_const_chars(vector* file, u32 current_index, \
const char* special_chars)
{
    #if DEBUG
    if (!is_special_char('\'', special_chars))
        send_error("\' has to be a special char for C const char replacement");
    if (!is_special_char('\\', special_chars))
        send_error("\\ has to be a special char for C const char replacement");
    #endif

    if (**(char**)vector_at(file, current_index, false) != '\'')
        return;

    u32 _initial_index = current_index;

    current_index += 1;

    /* This reads in the chars and turns them into a number. */
    u64 _result = 0;
    for (; current_index <= VECTOR_SIZE(*file)-1; current_index += 1) {
        find_next_valid_token(file, &current_index);

        // replace_C_escape_codes(file, &current_index);

        char* first_token = *(char**)vector_at(file, current_index, false);


        if (first_token == NULLPTR)
            continue;

        if (*first_token == '\'') {
            free(first_token);
            *(char**)vector_at(file, current_index, false) = NULLPTR;
            break;
        }

        _result += (u64)(*first_token);

        free(first_token);
        *(char**)vector_at(file, current_index, false) = NULLPTR;
    }

    /* This turns the "_result" into decimal chars. */
    u32 length = snprintf(NULL, 0, "%llu", (long long unsigned int)_result);
    char* destination = malloc(length + 1);
    CHECK_MALLOC(destination);
    snprintf(destination, length + 1, "%llu", (long long unsigned int)_result);
    *(char**)vector_at(file, _initial_index, false) = destination;
}

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * escape codes with their true values. This will replace the other tokens
 * inside of the backslash with null pointers. This doesn't check for NULL
 * pointers so it must be done before this is called.
 */
void replace_C_escape_codes(vector* file, u32* current_index, \
const char* special_chars)
{
    #if DEBUG
    if (!is_special_char('\\', special_chars))
        send_error("\\ has to be a special char for C backslash replacement");
    #endif
 
    // TODO: This shouldn't blindly increment "current_index" instead it should
    // find the next valid token.
    /*
     * "_next" is either the value of the escape code or the base if it's octal
     * or hex.
     */
    u8 _next = '\0';
    if (**(char**)vector_at(file, *current_index, false) == '\\') {  
        free(*(char**)vector_at(file, *current_index, false));
        switch (**(char**)vector_at(file, *current_index+1, false))
        {
        case 'n':
            _next = '\n';
            break;
        case 't':
            _next = '\t';
            break;
        case 'v':
            _next = '\v';
            break;
        case 'b':
            _next = '\b';
            break;
        case 'r':
            _next = '\r';
            break;
        case 'f':
            _next = '\f';
            break;
        case 'a':
            _next = '\a';
            break;
        case '\\':
            _next = '\\';
            break;
        case '\'':
            _next = '\'';
            break;
        case '\"':
            _next = '\"';
            break;
        case 'o':
            _next = 8;
            goto escape_codes_numerical_label;
            break;
        case 'x':
            _next = 16;
            goto escape_codes_numerical_label;
            break;
        default:
            printf("%s\n", *(char**)vector_at(file, *current_index+1, false));
            send_error("Unknown escape code");
        }

        /* This turns the backslash char into the escape code value. */
        // free(*(char**)vector_at(file, *current_index, false));
        **(char**)vector_at(file, *current_index, false) = _next;

        /* This frees the char after the backslash. */
        free(*(char**)vector_at(file, *current_index+1, false));
        *(char**)vector_at(file, *current_index+1, false) = NULL;
        return;

        escape_codes_numerical_label: ;

        /* This turns the number into an unsigned 64 bit int. */
        char* _begin = *(char**)vector_at(file, *current_index+1, false) + 1;
        u64 _result = strtoll(_begin, NULL, _next);

        /* This turns the "_result" into decimal chars. */
        u32 length = snprintf(NULL, 0, "%llu", (long long unsigned int)_result);
        char* destination = malloc(length+1);
        CHECK_MALLOC(destination)
        snprintf(destination, length + 1, "%llu", (long long unsigned int) \
            _result);
        destination[length] = '\0';

        free(_begin - 1);
        *(char**)vector_at(file, *current_index+1, false) = NULL;
        *(char**)vector_at(file, *current_index, false) = destination;
    }
}