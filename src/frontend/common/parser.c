/*
 * This contains a lot of commonly used parsing function branching a front end
 * to the intermediate stage.
 */

#include<frontend/common/parser.h>
#include<frontend/common/tokenizer.h>
#include<types.h>

static char* DEFAULT_INVALID_NAMES[] = { "if","fn", "let", "break", "return",
"loop", "\0" };

static char** INVALID_NAMES = DEFAULT_INVALID_NAMES;

/*
 * This parses and returns the type of the same name as the inputed string. This
 * assumes that the string is in a array and that there are no NULL pointers in
 * the array. This also assumes that the pointer char is a special char. If we
 * didn't get a type the returning type kind will be equal to 255.
 */
type parse_type(char** string_ptr)
{
    char** type_names = get_type_names();

    #if DEBUG
    if (!type_names)
        send_error("Type names weren't set");
    if (!(type_names[0xe]) && !(type_names[0xf][0]))
        send_error("Before and after pointer indicators cannot both be null");
    #endif

    u16 before_ptrs = 0;
    u16 after_ptrs = 0;
    type _type;

    for (; string_ptr[before_ptrs][0] == type_names[0xe][0]; before_ptrs++);

    string_ptr += before_ptrs;

    char* type_name = *string_ptr;

    _type.kind = 255;
    for (u32 i=0; (char)type_names[i][0] != '\0'; i++) {
        if (!strcmp(type_name, type_names[i])) {
            _type.kind = i;
            break;
        }
    }

    if (_type.kind == 255)
        return _type;

    string_ptr += 1;

    for (; string_ptr[after_ptrs][0] == type_names[0xf][0]; after_ptrs++);

    if (after_ptrs != before_ptrs && type_names[0xe][0] && type_names[0xf][0])
        send_error("Numbers of pointer chars before and afer must be equal");

    _type.ptr = after_ptrs;

    return _type;
}

/*
 * This returns the ASCII number of a string.
 */
i64 get_ascii_number(char* num_string)
{
    bool negative = num_string[0] == '-';
    i64 result = 0;
    for (u32 i = negative; i < strlen(num_string); i++) {
        result *= 10;
        result += (num_string[i] - 48);
    }

    if (negative)
        return -result;
    return result;
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

/*
 * If the inputed name is invalid it will return true. This is cap sensitive.
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

    /* Makes sure our "name" isn't a special character. */
    if(is_special_char(name[0]))
        return true;

    return false;
}

/*
 * This allows front ends to set custom invalid names. Along with the current
 * INVALID_NAMES being invalid, any special tokens, and types are counted as
 * invalid.
 */
void set_parser_invalid_names(char** _invalid_names)
{
    INVALID_NAMES = _invalid_names;
}