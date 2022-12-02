/*
 * This contains commonly used preprocessor functions.
 */

#include<frontend/common/preprocessor.h>

// TODO: C style comments don't nest so this function can be simplified.
/*
 * This function is meant to be run during the preprocessor loop and it skips
 * C style comments. This can leave the "current_index" pointing to the end of
 * the file, that has to be accounted for.
 */
void skip_C_comments(vector* file, u32* current_index)
{
    #if DEBUG
    if (!is_special_char('/') && !is_special_char('*'))
        send_error( \
            "'/' or '*' should be a special char for C style comment removal");
    if (is_white_space_char('/'))
        send_error("'/' can't be white space for C style comment removal");
    if (is_white_space_char('*'))
        send_error("'*' can't be white space for C style comment removal");
    #endif

    if (*current_index <= VECTOR_SIZE((*file))-2
    && (*(char**)vector_at(file, *current_index, false) == NULL
    || *(char**)vector_at(file, *current_index+1, false) == NULL))
        return;

    // TODO: Comments in comments are broken becaues /*/ would count as both
    // opening and closing a comment.
    u32 depth = 0;
    for (; *current_index <= VECTOR_SIZE((*file))-2; *current_index += 1) {

        char first_token = **(char**)vector_at(file, *current_index, false);
        char second_token = **(char**)vector_at(file, *current_index+1, false);

        if (first_token == '/' && second_token == '*')
            depth++;
        else if (!depth)
            return;

        free(*(char**)vector_at(file, *current_index, false));
        *(char**)vector_at(file, *current_index, false) = NULL;

        if (first_token == '*' && second_token == '/') {
            depth--;
            if (!depth) {
                free(*(char**)vector_at(file, *current_index+1, false));
                *(char**)vector_at(file, *current_index+1, false) = NULL;
                (*current_index) += 2;
                return;
            }
        }
    }
}

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * constant char strings with their constant values. This replaces other tokens
 * inside of the string with NULL pointers.
 */
void replace_C_const_chars(vector* file, u32 current_index)
{
    #if DEBUG
    if (!is_special_char('\''))
        send_error("\' has to be a special char for C const char replacment");
    if (!is_special_char('\\'))
        send_error("\\ has to be a special char for C const char replacment");
    #endif

    if (**(char**)vector_at(file, current_index, false) != '\'')
        return;

    u32 _initial_index = current_index;

    current_index += 1;

    /* This reads in the chars and turns them into a number. */
    u64 _result = 0;
    for (; current_index <= VECTOR_SIZE((*file))-1; current_index += 1) {
        find_next_valid_token(file, &current_index);

        // replace_C_escape_codes(file, &current_index);

        char* first_token = *(char**)vector_at(file, current_index, false);

        if (first_token == NULL)
            continue;

        _result += (u64)(*first_token);

        free(first_token);
        *(char**)vector_at(file, current_index, false) = NULL;
    }

    /* This turns the "_result" into decimal chars. */
    u32 length = snprintf(NULL, 0, "%llu", _result);
    char* destination = malloc(length + 1);
    if (destination == NULL)
        handle_error(0);
    snprintf(destination, length + 1, "%llu", _result);
    *(char**)vector_at(file, _initial_index, false) = destination;
}

/*
 * This function is meant to be run during the preprocessor loop and it replaces
 * escape codes with their true values. This will replace the other tokens
 * inside of the backslash with null pointers. This doesn't check for NULL
 * pointers so it must be done before this is called.
 */
void replace_C_escape_codes(vector* file, u32* current_index)
{
    #if DEBUG
    if (!is_special_char('\\'))
        send_error("\\ has to be a special char for C backslash replacment");
    #endif
 
    /*
     * "_next" is either the value of the escape code or the base if it's octal
     * or hex.
     */
    u8 _next = '\0';
    if (**(char**)vector_at(file, *current_index, false) == '\\') {  
        free(*(char**)vector_at(file, *current_index, false));
        find_next_valid_token(file, current_index);
        switch (**(char**)vector_at(file, *current_index+1, false)) {
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
            case '0':
                _next = '0';
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

        /* This turns the number into an unsigned 128bit. */
        char* _begin = *(char**)vector_at(file, *current_index+1, false) + 1;
        u64 _result = strtoll(_begin, NULL, _next);

        /* This turns the "_result" into decimal chars. */
        u32 length = snprintf(NULL, 0, "%llu", _result);
        char* destination = malloc(length+1);
        if (destination == NULL)
            handle_error(0);
        snprintf(destination, length + 1, "%llu", _result);
        destination[length] = '\0';

        free(_begin - 1);
        *(char**)vector_at(file, *current_index+1, false) = NULL;
        *(char**)vector_at(file, *current_index, false) = destination;
    }
}