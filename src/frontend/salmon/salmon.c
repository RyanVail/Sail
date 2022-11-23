/*
 * This is the front end for the Salmon programming language. TODO: This file 
 * should be named parser or something along those lines.
 */

#include<frontend/salmon/salmon.h>
#include<frontend/salmon/preprocessor.h>
#include<backend/intermediate/symboltable.h>
#include<frontend/common/parser.h>

static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location);
static inline void salmon_parse_while(vector* file, u32* location);
static inline void salmon_parse_else(vector* file, u32* location);
static inline void salmon_parse_let(vector* file, u32* location);
static inline void salmon_parse_if(vector* file, u32* location);
static inline void salmon_parse_fn(vector* file, u32* location);

// #
    // #include
    // #asm
    // #extern // #intern
        // Inputs
            // Name
                // Return
// Let
    // Type
        // Name
// Fn
    // Inputs
        // Name
            // Return
// If
    // {
    // Operation
    // Else
        // {
        // Operation
// While

/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns a vector of those intermediate tokens.
 */
vector salmon_file_into_intermediate(char* file_name)
{
    vector file = salmon_preprocess_file(file_name);
    for (u32 i=0; i < file.apparent_size; i++) {
        // printf("%s\n", *(char**)vector_at(&file, i, false));
        if (salmon_parse_initial_syntax_tree(&file, &i))
            continue;
    }
    return file;
}

// TODO: A lot of these functions don't check for the end of the file so they
// have a chance of seg faulting if we do reach the end of the file. 
/*
 * This function parses through the initial salmon syntax tree by uncapitalizing
 * the initial keyword and calling the needed functions. This returns true if it
 * does read something.
 */
static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location)
{
    /* This takes the token copies it and uncapitalizes it. */
    char* _token = malloc(strlen(*(char**)vector_at(file, *location, false)));
    if (_token == NULL)
        handle_error(0);
    strcpy(_token, *(char**)vector_at(file, *location, false));
    strlower(_token);

    if (!strcmp(_token, "if"))
        salmon_parse_if(file, location);
    else if (!strcmp(_token, "else"))
        salmon_parse_else(file, location);
    else if (!strcmp(_token, "fn"))
        salmon_parse_fn(file, location);
    else if (!strcmp(_token, "let"))
        salmon_parse_let(file, location);
    else if (!strcmp(_token, "while"))
        salmon_parse_while(file, location);
    else {
        free(_token);
        return false;
    }

    free(_token);
    return true;
}

/*
 * These functions are all secondary branches on the syntax tree.
 */
inline void salmon_parse_if(vector* file, u32* location)
{
    //
}
inline void salmon_parse_else(vector* file, u32* location)
{
    //
}
inline void salmon_parse_fn(vector* file, u32* location)
{
    *location += 1;
    
    if (**(char**)vector_at(file, *location, false) == '$')
        goto salmon_parse_fn_read_fn_name_label;

    /* This reads through the input variables. */
    vector inputs = { 0, 0, 0, sizeof(type) };
    for (; *location < VECTOR_SIZE((*file)); *location += 1) {
        char* _name = *(char**)vector_at(file, *location, false);
        if (is_invalid_name(_name))
            send_error("Invalid input variable name");

        *location += 1;

        type _type = parse_type((char**)vector_at(file, *location, false));
        if (_type.kind == 255)
            send_error("Unknown type in function inputs");

        *location += 1;

        // TODO: ADD VARIABLE TO SYMBOL TABLE HERE!
        vector_append(&inputs, &_type);
        if (**(char**)vector_at(file, *location, false) != ',')
            break;
    }

    if (**(u16**)vector_at(file, *location, false) != ('<' | ('-' << 8)))
        send_error("Expected \"<-\" after function inputs");

    /* This reads the function name. */
    salmon_parse_fn_read_fn_name_label:
    *location += 1;
    if (**(char**)vector_at(file, *location, false) != '$')
        send_error("Expected a '$' before function name");

    *location += 1;
    char* fn_name = *(char**)vector_at(file, *location, false);
    if (is_invalid_name(fn_name))
        send_error("Function name is not valid");

    *location += 1;
    if (**(char**)vector_at(file, *location, false) != '$')
        send_error("Expected a '$' after function name");

    /* This reads the function return type. */
    *location += 1;
    type return_type = { 0, 255 };
    if (**(u16**)vector_at(file, *location, false) == ('-' | ('>' << 8)))
        return_type = parse_type((char**)vector_at(file, *location+1, false));

    if (!add_function_symbol(fn_name, inputs, return_type, 0))
        send_error("Function name already used.");
}
inline void salmon_parse_let(vector* file, u32* location)
{
    /* This is the variable name. */
    *location += 1;
    if (is_invalid_name(*(char**)vector_at(file, *location, false)))
        send_error("Invalid name");
    char* name = *(char**)vector_at(file, *location, false);

    /* This is the variable type. */
    *location += 1;
    type _type = parse_type((char**)vector_at(file, *location, false));
    if (_type.kind == 255)
        send_error("Expected type after let");

    add_variable_symbol(name, _type, 0);
    // add inter variable decleration
}
inline void salmon_parse_while(vector* file, u32* location)
{
    //
}