/*
 * This contains a lot of commonly used parsing function connecting a frontend
 * to the intermediate stage.
 */

#include<intermediate/typedef.h>
#include<intermediate/enum.h>
#include<intermediate/struct.h>
#include<datastructures/hash_table.h>
#include<frontend/common/parser.h>
#include<frontend/common/tokenizer.h>
#include<types.h>

/*
 * This handles common errors that come up and shouldn't be set to the main
 * error handler, rather be called on specific error types.
 */
error_token_range parser_handle_error(intermediate_pass* _pass, \
parsing_error _error, char** token)
{
    error_token_range _range;
    memset(&_range, 0, sizeof(error_token_range));

    vector _vec;
    bool last_char_was_error;
    char* _token;
    char _char;
    #if DESCRIPTIVE_ERRORS
    invalid_name_type _type;
    #endif

    switch (_error)
    {
    case PARSING_ERROR_INVALID_VAR_NAME:
        #if DESCRIPTIVE_ERRORS
        _type = get_why_invalid_name(_pass, *token);
        /* Printing the full string reasons a name can be invalid. */
        if (_type & INVALID_NAME_TYPE_IS_IN_INVALID_NAMES) {
            printf("%s Name is invalid: %s\n", ERROR_STRING, *token);
        } else if (_type & INVALID_NAME_TYPE_IS_A_TYPE) {
            printf("%s Name is already the name of a type: %s\n", ERROR_STRING, \
            *token);
        } else if (_type & INVALID_NAME_TYPE_STARTS_WITH_NUMBER) {
            printf("%s Invalid name, starts with a number\n", ERROR_STRING);
        }

        _range.type = ERROR_RANGE_TYPE_FAILED;
        _range.starting_token = token;
        _range.ending_token = token;

        if (!(_type & 0x3))
            return _range;

        /* This vector is just used to hold the colored token. */
        _vec = vector_init(1, 4);

        _token = *token;

        /* This is quick and dirty macro to add a str to a vector of chars. */
        #define ADD_STR_TO_VEC(_vec, _str) ( \
            { \
                for (u32 i=0; _str[i] != '\0'; i++) \
                    vector_append((_vec), &_str[i]); \
            } \
        )

        /* Creating the error token with the highlighting. */
        for (_char = *_token; _char != '\0'; _token++) {
            _char = *_token;
            if (is_special_char(_char, _pass->data.front_end->special_chars) \
            || ('a' <= _char && _char <= '9' && _char == *_token)) {
                if (!last_char_was_error)
                    ADD_STR_TO_VEC(&_vec, ERROR_COLOR_START);
                vector_append(&_vec, &_char);
                last_char_was_error = true;
            } else {
                if (last_char_was_error)
                    ADD_STR_TO_VEC(&_vec, COLOR_END);
                vector_append(&_vec, &_char);
                last_char_was_error = false;
            }
        }

        if (last_char_was_error)
            ADD_STR_TO_VEC(&_vec, COLOR_END);

        #undef ADD_STR_TO_VEC

        _range.overide_token = _vec.contents;

        #else
        printf("%s The name: %s is invalid", ERROR_STRING, *token);
        #endif
        break;
    }

    return _range;
}

#if DESCRIPTIVE_ERRORS
// TODO: This and invalid name should check for typedefs instead of the
// functions that call it doing it seperatly.
/*
 * This function and enum is used by "parser_handle_error" to give descriptive
 * errors on invalid names.
 */
invalid_name_type get_why_invalid_name(intermediate_pass* _pass, char* name)
{
    invalid_name_type result = 0;

    #if DEBUG
    if (_pass->data.front_end == NULLPTR)
        send_error("Intermediate pass' front end is not set");
    if (_pass->data.front_end->type_names == NULLPTR)
        send_error("Front end's type names aren't set");
    #endif

    /* Getting data from the front ends. */
    char** invalid_names = _pass->data.front_end->invalid_names;
    char** types = _pass->data.front_end->type_names;

    /* Checks if the first letter is a number. */
    if ('a' <= name[0] && name[0] <= '9')
        result |= INVALID_NAME_TYPE_STARTS_WITH_NUMBER;

    /* Checks if the "name" matches anything in invalid names. */
    for (u32 i=0; invalid_names[i][0] != '\0'; i++)
        if (!strcmp(name, invalid_names[i]))
            result |= INVALID_NAME_TYPE_IS_IN_INVALID_NAMES;

    /* Checks if the "name" matches any type names. */
    for (u32 i=0; (char)types[i][0] != '\0'; i++)
        if (!strcmp(name, types[i]))
            result |= INVALID_NAME_TYPE_IS_A_TYPE;

    /* Makes sure "name" doesn't have any special character. */
    for (u32 i=0; name[i] != '\0'; i++)
        if (is_special_char(name[i], _pass->data.front_end->special_chars))
            result |= INVALID_NAME_TYPE_INVALID_CHAR;

    return result;
}
#endif

// TODO: This should be called read_type or something.
// TODO: It might be better for this to take in a ptr to the index.
/*
 * This parses and returns the type of the same name as the inputted string.
 * This will also read and set the ptr count of the returned type. This assumes
 * that the string is in a array and that there are no NULL pointers in the
 * array. This also assumes that the pointer char is a special char. If there's
 * no type a type the returned type's kind will be NO_TYPE. This sets errno on
 * errors. This also sets errno_value to the "inital_token" appon returning
 * NO_TYPE and if type ptrs are unequal.
 */
type get_type(intermediate_pass* _pass, vector* file, u32 index)
{
    type _type = {
        .kind = NO_TYPE,
        .ptr_count = 0,
        .extra_data = 0,
    };

    if (IS_VEC_END(*file, index))
        return _type;

    #if DEBUG
    if (_pass->data.front_end != NULLPTR)
        if (_pass->data.front_end->type_names == NULLPTR)
            send_error("Front end's type names is not set");
    #endif

    #if DESCRIPTIVE_ERRORS
    /* This is used in case of an error. */
    char** inital_token = &VECTOR_AT(file, index, char*);
    #endif

    /* This is here for code readability. */
    char** type_names = _pass->data.front_end->type_names;

    /* The current token. */
    char* token = *inital_token;

    i32 before_ptrs = 0;
    i32 after_ptrs = 0;

    /* Reading the starter pointer chars. */
    while (token[0] != 0 && *token == *type_names[TYPE_NAME_FIRST_PTR_INDEX]) {
        find_next_valid_token(file, &index);
        before_ptrs++;
        token = VECTOR_AT(file, index, char*);
    }

    _type.ptr_count = before_ptrs;
    char* type_name = token;

    // TODO: This is in kinda a dumb place because it's after the reading of the
    // first ptr chars.
    /* Attempting to read the type from the front end. */
    if (_pass->data.front_end->type_reader_func != NULLPTR) {
        _type = (*_pass->data.front_end->type_reader_func)(_pass, index);
        if (_type.kind != NO_TYPE)
            return _type;
    }

    /* Setting the type_kind. */
    _type.kind = NO_TYPE;
    for (u32 i=0; (char)type_names[i][0] != '\0'; i++) {
        if (!strcmp(type_name, type_names[i])) {
            _type.kind = i;
            break;
        }
    }

    // TODO: This is terribly structured.
    /* If the type isn't found, check if we're reading a struct. */
    if (_type.kind == NO_TYPE) {
        HASH_STRING(type_name);
        intermediate_struct* _struct = get_struct(_pass, result_hash);
        if (_struct != NULLPTR) {
            _type.kind = STRUCT_TYPE;
            _type.extra_data = _struct->hash;
        } else {
            /* If it isn't a struct check if it's a typedef. */
            intermediate_typedef* _typedef = get_typedef(&_pass->typedefs, \
                result_hash);
            if (_typedef == NULLPTR) {
                #if DESCRIPTIVE_ERRORS
                if (_type.kind == NO_TYPE)
                    stack_push(&error_value, inital_token);
                #endif
                return _type;
            }
            _type = _typedef->type;
        }
    }

    /* Reading the ending pointer chars. */
    after_ptrs--;
    do {
        after_ptrs++;
        find_next_valid_token(file, &index);
        token = VECTOR_AT(file, index, char*);
    } while (token[0] != 0 && *token == *type_names[TYPE_NAME_LAST_PTR_INDEX]);

    /* Making sure the pointer chars before and after are equal. */
    if (after_ptrs != before_ptrs && type_names[TYPE_NAME_FIRST_PTR_INDEX][0] \
    && type_names[TYPE_NAME_LAST_PTR_INDEX][0]) {
        errno = PARSING_ERROR_TYPE_PTRS_UNEQUAL;
        #if DESCRIPTIVE_ERRORS
        stack_push(&error_value, inital_token);
        #endif
    }

    #if DESCRIPTIVE_ERRORS
    if (_type.kind == NO_TYPE)
        stack_push(&error_value, inital_token);
    #endif

    return _type;
}

/*
 * This goes through the inputted string and returns 0 if it isn't a float, 1 if
 * it is a float, and 2 if it's a double indicated by the trailing 'd' or 'f',
 * but defaulting to a float.
 */
is_ascii_float_return is_ascii_float(char** float_token)
{
    // TODO: If "is_ascii_float" and "get_ascii_float" used indexes rather than
    // ptrs that would get rid of this terribleness.
    /* This is used to tell how many tokens there are in this float. */
    char** starting_token = float_token;

    bool found_period = false;
    char* current_token = *float_token;
    char current_char;
    u8 float_type = 0;
    while (true) {
        /* Making sure this token is valid. */
        if (current_token == NULLPTR) {
            float_token++;
            current_token = *float_token;
            continue;
        }

        current_char = *current_token;

        /* Checking for the period. */
        if (current_char == '.') {
            if (found_period) {
                float_type = 0;
                goto is_ascii_float_return_construct_struct_label;
            }
            found_period = true;
            float_token++;
            current_token = *float_token;
            continue;
        }

        /* Checking for trailing 'f' and 'd' chars. */
        if (current_char == 'f' || current_char == 'd') {
            if (*(current_token+1) != '\0') {
                float_type = 0;
                goto is_ascii_float_return_construct_struct_label;
            }
            float_type = (current_char == 'f') ? (1) : (2);
            goto is_ascii_float_return_construct_struct_label;
        }

        /* Checking if the string is done. */
        if (current_char == '\0') {
            /* If there's a period it's done otherwise go to the next token. */
            if (found_period) {
                float_type = 1;
                goto is_ascii_float_return_construct_struct_label;
            } else {
                float_token++;
                current_token = *float_token;
                continue;
            }
        }

        /* If this character isn't a number, this isn't a float. */
        if ('0' > current_char || current_char > '9') {
            float_type = 0;
            goto is_ascii_float_return_construct_struct_label;
        }

        current_token++;
    }

    is_ascii_float_return_construct_struct_label: ;

    is_ascii_float_return result = {
        .end_ptr = float_token,
        .token_length = (u32)(size_t)(float_token - starting_token),
        .type = float_type,
    };

    return result;
}

/*
 * This returns the f64 representation of the inputted token.
 */
f64 get_ascii_float(char** float_token, char** ending_float_token)
{
    f64 float_value = 0.0f;
    bool found_period = false;
    char* current_token;
    while (true) {
        /* Getting this token. */
        current_token = *float_token;
        if (current_token == NULLPTR)
            continue;

        /* Checking for a period. */
        if (*current_token == '.')
            found_period = true;

        /* Getting this token's value and dividing it if there was a period. */
        f64 tmp_value = strtold(current_token, NULLPTR);
        if (found_period)
            for (int i=0; i < strlen(current_token) - 1; i++)
                tmp_value /= 10;
        float_value += tmp_value;

        /* Checking if this is the last token. */
        if (float_token == ending_float_token)
            return float_value;

        /* Incrementing to the next token. */
        float_token++;
    }
}

/*
 * This returns the numeral value of an ASCII string. This supports negative
 * signs but doesn't support suffix or prefixes.
 */
num get_ascii_number(char* num_string)
{
    num result = {
        .magnitude = 0,
        .negative = (num_string[0] == '-'),
    };

    u32 num_len = strlen(num_string);
    for (u32 i = (u32)result.negative; i < num_len; i++) {
        result.magnitude *= 10;
        result.magnitude += (num_string[i] - '0');
    }

    return result;
}

/*
 * Returns true if the inputted number string contains all number characters
 * this supports negative numbers and suffixes, but doesn't support numbers with
 * decimal points. Prefixes and suffixes both need to be one character and
 * prefixes are assumed to start with a '0' so that has to be excluded from the
 * inputted "prefixes" string of chars.
 */
bool is_ascii_number(char* num_string, const char* prefixes,
const char* suffixes)
{
    bool read_number = false;
    bool has_suffix = false;
    for (u32 i=(u32)(num_string[0] == '-'); i < strlen(num_string); i++) {
        if (num_string[i] >= '0' && num_string[i] <= '9') {
            if (has_suffix)
                return false;

            read_number = true;
            continue;
        }

        /* Checking for "0{prefix_char}" prefixes. */
        if (i == 1 && num_string[0] == '0') {
            for (const char* _prefix = prefixes; *_prefix != '\0'; _prefix++)
                if (*_prefix == num_string[i])
                    goto is_ascii_number_continue_reading_label;

            return false;
        }

        /* Checking if this number has a suffix. */
        for (const char* _suffix = suffixes; *_suffix != '\0'; _suffix++) {
            if (*_suffix == num_string[i]) {
                has_suffix = true;
                goto is_ascii_number_continue_reading_label;
            }
        }

        return false;

        is_ascii_number_continue_reading_label: ;
    }

    /*
     * Read number is used here because if the number is just a prefix and or a
     * suffix and no number this function would return true which is shouldn't.
     */
    return read_number;
}

// TODO: This function should test hashes against hashes.
/*
 * If the inputted name is invalid it will return true. This is cap sensitive.
 * Any names found in the inputted intermediate pass' front end's invalid_names
 * and type_names are considered invalid. If the name starts with a number it is
 * also considered invalid.
 */
bool is_invalid_name(intermediate_pass* _pass, char* name)
{
    #if DEBUG
    if (_pass->data.front_end == NULLPTR)
        send_error("Intermediate pass' front end is not set");
    if (_pass->data.front_end->type_names == NULLPTR)
        send_error("Front end's type names aren't set");
    if (_pass->data.front_end->invalid_names == NULLPTR)
        send_error("Front end's invalid names aren't set");
    #endif

    /* Getting data from the front ends. */
    char** invalid_names = _pass->data.front_end->invalid_names;
    char** types = _pass->data.front_end->type_names;

    /* Checks if the first letter is a number. */
    if ('0' <= name[0] && name[0] <= '9')
        return true;

    /* Checks if the "name" matches anything in invalid names. */
    for (u32 i=0; invalid_names[i][0] != '\0'; i++)
        if (!strcmp(name, invalid_names[i]))
            return true;

    /* Checks if the "name" matches any type names. */
    for (u32 i=0; (char)types[i][0] != '\0'; i++)
        if (!strcmp(name, types[i]))
            return true;

    /* Makes sure "name" doesn't have any special character. */
    for (u32 i=0; name[i] != '\0'; i++)
        if (is_special_char(name[i], _pass->data.front_end->special_chars))
            return true;

    return false;
}

/*
 * This goes from the current position in the file till it reaches a '\n' and
 * returns the file index of the '\n'.
 */
u32 get_end_of_line(vector* file, u32 i)
{
    char _char;
    for (; i < VECTOR_SIZE(*file)-1; i++)
        // TODO: Is this the desired behaviour?
        if (VECTOR_AT(file, i, char*) == NULLPTR)
            return i;

        _char = *VECTOR_AT(file, i, char*);

        if (_char == '\n' || _char == ';')
            return i;

    return i;
}