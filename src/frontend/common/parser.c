/*
 * This contains a lot of commonly used parsing function branching a frontend
 * to the intermediate stage.
 */
// TODO: This should split the lines into their own struct like below to make
// the whole parsing process a lot easier.
/*
struct token_line {
    char** tokens;
}
*/

#include<intermediate/typedef.h>
#include<intermediate/enum.h>
#include<intermediate/struct.h>
#include<datastructures/hash_table.h>
#include<frontend/common/parser.h>
#include<frontend/common/tokenizer.h>
#include<types.h>

static char* DEFAULT_INVALID_NAMES[] = { "if", "fn", "let", "break", "return",
"loop", "\0" };

static char** INVALID_NAMES = DEFAULT_INVALID_NAMES;

/*
 * This handles common errors that come up and shouldn't be set to the main
 * error handler, rather be called on specific error types.
 */
error_token_range parser_handle_error(parsing_error _error, char** token)
{
    error_token_range _range;
    memset(&_range, 0, sizeof(error_token_range));

    vector _vec;
    invalid_name_type _type;
    bool last_char_was_error;
    char* _token;
    char _char;

    switch (_error)
    {
    case PARSING_ERROR_INVALID_VAR_NAME:
        #if DESCRIPTIVE_ERRORS
        _type = get_why_invalid_name(*token);
        /* Printing the full string reasons a name can be invalid. */
        if (_type & INVALID_NAME_TYPE_IS_IN_INVALID_NAMES) {
            printf("%s Name is invalid: %s\n", ERROR_STRING, *token);
        } else if (_type & INVALID_NAME_TYPE_IS_A_TYPE) {
            printf("%s Name is already the name of a type: %s\n", ERROR_STRING, \
            *token);
        } else if (_type & INVALID_NAME_TYPE_IS_MODIFIER) {
            printf("%s Name is already the name of a type modifier: %s\n", \
            ERROR_STRING, *token);
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
        #define ADD_STR_TO_VEC(_vec, _str) \
            for (u32 i=0; _str[i] != '\0'; i++) { \
                vector_append((_vec), &_str[i]); \
            }

        /* Creating the error token with the highlighting. */
        for (_char = *_token; _char != '\0'; _token++) {
            _char = *_token;
            if (is_special_char(_char) || (48 <= _char && _char <= 57 && \
            _char == *_token)) {
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
invalid_name_type get_why_invalid_name(char* name)
{
    invalid_name_type result = 0;

    /* Checks if the first letter is a number. */
    if (48 <= name[0] && name[0] <= 57)
        result |= INVALID_NAME_TYPE_STARTS_WITH_NUMBER;

    /* Checks if the "name" matches anything in invalid names. */
    for (u32 i=0; INVALID_NAMES[i][0] != '\0'; i++)
        if (!strcmp(name, INVALID_NAMES[i]))
            result |= INVALID_NAME_TYPE_IS_IN_INVALID_NAMES;

    /* Checks if the "name" matches any type names. */
    char** _types = get_type_names();
    for (u32 i=0; (char)_types[i][0] != '\0'; i++)
        if (!strcmp(name, _types[i]))
            result |= INVALID_NAME_TYPE_IS_A_TYPE;

    /* Checks if the "name" matches any type modifier names. */
    char** _modifier = get_type_modifier_names();
    for (u32 i=0; (char)_types[i][0] != '\0'; i++)
        if (!strcmp(name, _types[i]))
            result |= INVALID_NAME_TYPE_IS_MODIFIER;

    /* Makes sure "name" doesn't have any special character. */
    for (u32 i=0; name[i] != '\0'; i++)
        if (is_special_char(name[i]))
            result |= INVALID_NAME_TYPE_INVALID_CHAR;

    return result;
}
#endif

/*
 * This parses and returns the given type modifiers. This will increment token
 * till it reaches the end of the modifiers. unsigned and signed modifiers will
 * change the first bit of the returning kind. This will skip tokens that are
 * equal to NULLPTR and returns when it doesn't hit a modifier.
 */
type_kind get_type_modifier(char*** token)
{
    char** modifier_names = get_type_modifier_names();
    type_kind _type;
    while (true) {
        if (**token == NULLPTR)
            continue;
        for (u32 i=0; modifier_names[i][0] != '\0'; i++) {
            if (!strcmp(modifier_names[i],**token)) {
                switch (i)
                {
                case 0: // Unsigned
                    _type |= 1;
                    goto get_type_modifier_next_token_label;
                case 1: // Signed
                    _type &= 0b1111111111111111;
                    goto get_type_modifier_next_token_label;
                default: // Other flags
                    _type &= 1 << i + 4;
                    goto get_type_modifier_next_token_label;
                }
            }
            return _type;
        }
        get_type_modifier_next_token_label:
        **token += 1;
    }
}

// TODO: 255 should be replace with "__UINT8_MAX__".
/*
 * This parses and returns the type of the same name as the inputted string.
 * This will also read and set the ptr count of the returned type. This assumes
 * that the string is in a array and that there are no NULL pointers in the
 * array. This also assumes that the pointer char is a special char. If there's
 * no type a type the returned type's kind will be equal to 255. This sets errno
 * on errors. This also sets errno_value to the "inital_token" appon returning
 * 255 and if type ptrs are unequal.
 */
type get_type(char** token)
{
    /* This is used in case of an error. */
    char** inital_token = token;

    char** type_names = get_type_names();

    #if DEBUG
    if (!type_names)
        send_error("Type names weren't set");
    if (!(type_names[0xc]) && !(type_names[0xd][0]))
        send_error("Before and after pointer indicators cannot both be null");
    #endif

    u16 before_ptrs = 0;
    u16 after_ptrs = 0;
    type _type;

    /* Reading the starter pointer chars. */
    while (token[before_ptrs][0] == type_names[0xd][0])
        before_ptrs++;

    token += before_ptrs;

    char* type_name = *token;

    /* Setting the type_kind. */
    _type.kind = 255;
    for (u32 i=0; (char)type_names[i][0] != '\0'; i++) {
        if (!strcmp(type_name, type_names[i])) {
            _type.kind = i;
            break;
        }
    }

    /* If the type isn't found, check if we're reading a struct. */
    if (_type.kind == 255) {
        HASH_STRING(type_name);
        intermediate_struct* _struct = get_struct(result_hash);
        if (_struct != NULLPTR) {
            _type.kind = STRUCT_TYPE;
            _type.ptr = _struct->hash;
        } else {
            /* If it isn't a struct check if it's a typedef. */
            intermediate_typedef* _typedef = get_typedef(result_hash);
            if (_typedef == NULLPTR) {
                #if DESCRIPTIVE_ERRORS
                if (_type.kind == 255)
                    stack_push(&error_value, inital_token);
                #endif
                return _type;
            }
            _type = _typedef->type;
        }
    }

    token += 1;

    /* Reading the ending pointer chars. */
    while (token[after_ptrs][0] == type_names[0xd][0])
        after_ptrs++;

    /* Making sure the pointer chars before and after are equal. */
    if (after_ptrs != before_ptrs && type_names[0xc][0] && type_names[0xd][0]) {
        errno = PARSING_ERROR_TYPE_PTRS_UNEQUAL;
        #if DESCRIPTIVE_ERRORS
        stack_push(&error_value, inital_token);
        #endif
    }

    /* Setting the pointer counters. */
    if (IS_TYPE_STRUCT(_type))
        _type.kind = (_type.kind << 16 >> 16) + (after_ptrs << 16);
    else
        _type.ptr = after_ptrs;

    #if DESCRIPTIVE_ERRORS
    if (_type.kind == 255)
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
        if (48 > current_char || current_char > 57) {
            float_type = 0;
            goto is_ascii_float_return_construct_struct_label;
        }

        current_token++;
    }

    is_ascii_float_return_construct_struct_label: ;

    is_ascii_float_return returning = { float_token, \
    float_token - starting_token, float_type };
    return returning;
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
 * This returns the numeral value of an ASCII string.
 */
i64 get_ascii_number(char* num_string)
{
    bool negative = num_string[0] == '-';
    i64 result = 0;
    for (u32 i = negative; i < strlen(num_string); i++) {
        result *= 10;
        result += (num_string[i] - 48);
    }

    return negative ? -result : result;
}

/*
 * This goes through a string and returns true if it is an ASCII number.
 */
bool is_ascii_number(char* num_string)
{
    bool negative = num_string[0] == '-';
    for (u32 i=(u32)negative; i < strlen(num_string); i++)
        if (48 > num_string[i] || num_string[i] > 57)
            return false;

    return true;
}

// TODO: This function should test hashes against hashes.
/*
 * If the inputted name is invalid it will return true. This is cap sensitive.
 * Along with the current INVALID_NAMES being invalid, any special tokens, and
 * types are counted as invalid. If the name starts with a number it is also
 * considered invalid.
 */
bool is_invalid_name(char* name)
{
    /* Checks if the first letter is a number. */
    if (48 <= name[0] && name[0] <= 57)
        return true;

    /* Checks if the "name" matches anything in invalid names. */
    for (u32 i=0; INVALID_NAMES[i][0] != '\0'; i++)
        if (!strcmp(name, INVALID_NAMES[i]))
            return true;

    /* Checks if the "name" matches any type names. */
    char** _types = get_type_names();
    for (u32 i=0; (char)_types[i][0] != '\0'; i++)
        if (!strcmp(name, _types[i]))
            return true;

    /* Checks if the "name" matches any type modifier names. */
    char** _modifier = get_type_modifier_names();
    for (u32 i=0; (char)_types[i][0] != '\0'; i++)
        if (!strcmp(name, _types[i]))
            return true;

    /* Makes sure "name" doesn't have any special character. */
    for (u32 i=0; name[i] != '\0'; i++)
        if (is_special_char(name[i]))
            return true;

    return false;
}

/*
 * This goes from the current position in the file till it reaches a '\n' and
 * returns the file index of the '\n'.
 */
u32 get_end_of_line(vector* file, u32 i)
{
    for (; i < VECTOR_SIZE((*file)); i++)
        if (**(char**)vector_at(file, i, 0) == '\n')
            return i;

    return i;
}

/*
 * This allows frontends to set custom invalid names. Along with the current
 * INVALID_NAMES being invalid, any special tokens, and types are counted as
 * invalid.
 */
void set_parser_invalid_names(char** _invalid_names)
{
    INVALID_NAMES = _invalid_names;
}