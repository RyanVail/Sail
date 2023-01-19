/*
 * This contains a lot of commonly used parsing function branching a front end
 * to the intermediate stage.
 */

#include<backend/intermediate/struct.h>
#include<datastructures/hashtable.h>
#include<frontend/common/parser.h>
#include<frontend/common/tokenizer.h>
#include<types.h>

static char* DEFAULT_INVALID_NAMES[] = { "if", "fn", "let", "break", "return",
"loop", "\0" };

static char** INVALID_NAMES = DEFAULT_INVALID_NAMES;

/*
 * This parses and returns the given type modifiers. This will incrament token
 * till it reaches the end of the modifiers. unsigned and signed modifiers will
 * change the first bit of the returinging kind. This will skip tokens that are
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
                    _type &= 0b01111111111111111;
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
 * This parses and returns the type of the same name as the inputed string. This
 * assumes that the string is in a array and that there are no NULL pointers in
 * the array. This also assumes that the pointer char is a special char. If we
 * didn't get a type the returning type kind will be equal to 255.
 */
type get_type(char** token)
{
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

    for (; token[before_ptrs][0] == type_names[0xd][0]; before_ptrs++);

    token += before_ptrs;

    char* type_name = *token;

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
        intermediate_struct* _struct = find_struct(result_hash);
        if (_struct == NULLPTR)
            return _type;
        _type.kind = STRUCT_TYPE;
        _type.ptr = _struct->hash;
    }

    token += 1;

    for (; token[after_ptrs][0] == type_names[0xd][0]; after_ptrs++);

    if (after_ptrs != before_ptrs && type_names[0xc][0] && type_names[0xd][0])
        send_error("Numbers of pointer chars before and afer must be equal");

    if (IS_TYPE_STRUCT(_type))
        _type.kind = (_type.kind << 16 >> 16) + (after_ptrs << 16);
    else
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
 * This allows front ends to set custom invalid names. Along with the current
 * INVALID_NAMES being invalid, any special tokens, and types are counted as
 * invalid.
 */
void set_parser_invalid_names(char** _invalid_names)
{
    INVALID_NAMES = _invalid_names;
}