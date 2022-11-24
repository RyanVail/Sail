/*
 * This is the front end for the Salmon programming language. TODO: This file 
 * should be named parser or something along those lines.
 */

#include<frontend/salmon/salmon.h>
#include<frontend/common/parser.h>
#include<frontend/salmon/preprocessor.h>

#include<backend/intermediate/symboltable.h>
#include<backend/intermediate/intermediate.h>

static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location);
static inline void salmon_parse_while(vector* file, u32* location);
static inline void salmon_parse_else(vector* file, u32* location);
static inline void salmon_parse_let(vector* file, u32* location);
static inline void salmon_parse_if(vector* file, u32* location);
static inline void salmon_parse_fn(vector* file, u32* location);
bool salmon_parse_operation(char* string);

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

// TODO: Freeing the file that is returned from this function will replace
// the names in the symbol table with junk.
/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns a vector of those intermediate tokens.
 */
vector salmon_file_into_intermediate(char* file_name)
{
    vector file = salmon_preprocess_file(file_name);
    for (u32 i=0; i < file.apparent_size; i++) {
        char* current_token = *(char**)vector_at(&file, i, false);
        // printf("%s\n", *(char**)vector_at(&file, i, false));
        if (salmon_parse_initial_syntax_tree(&file, &i))
            continue;

        if (salmon_parse_operation(current_token))
            continue;

        if (!is_invalid_name(current_token)
        && get_variable_symbol(current_token, 0)) {
            intermediate _operand = { VAR_ACCESS,\
                get_variable_symbol(current_token, 0) };

            add_operand(_operand, false);
        }

        if (is_ascii_number(current_token)) {
            // TODO: This can be removed with a "#if" on 64 bit.
            // TODO: Large constants should be stored in a vector rather than
            // seperatly.
            i128 _const_num = get_ascii_number(current_token);
            intermediate _operand;
            void* const_num;
            /*
             * If we have an i128 larger than the size of a pointer we store the
             * value on the heap and make the ptr point to the value.
             */
            if (_const_num > ~((i128)1 << ((sizeof(void*) << 3) - 1))
            || _const_num < (i128)1 << ((sizeof(void*) << 3) - 1)) {
                const_num = malloc(sizeof(i128));
                if (const_num == 0)
                    handle_error(0);
                memcpy(const_num, &_const_num, sizeof(i128));
                intermediate _operand = { CONST_PTR, const_num };
            } else {
                const_num = (void*)_const_num;
                intermediate _operand = { CONST, const_num };
            }
            add_operand(_operand, false);
        }
        // printf("%s\n", *(char**)vector_at(&file, i, false));
    }
    // clear_operand_stack();
    return file;
}

// TODO: A lot of these functions don't check for the end of the file so they
// have a chance of seg faulting if we do reach the end of the file. 
/*
 * This function parses through the initial salmon syntax tree by reading the
 * initial keyword and calling the needed functions. This returns true if it
 * does read something.
 */
static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location)
{
    char* _token = *(char**)vector_at(file, *location, false);
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
    else
        return false;

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

/*
 * This parses through single character operations and adds the operations the
 * intermediates. Returns true if it read something.
 */
static inline bool salmon_parse_single_char_operation(char _char)
{
    intermediate_type _operation;
    switch (_char) {
        case '+':
            _operation = ADD;
            break;
        case '-':
            _operation = SUB;
            break;
        case '/':
            _operation = DIV;
            break;
        case '*':
            _operation = MUL;
            break;
        case '^':
            _operation = XOR;
            break;
        case '|':
            _operation = OR;
            break;
        case '&':
            _operation = ADD;
            break;
        case '%':
            _operation = MOD;
            break;
        case '~':
            _operation = COMPLEMENT;
            break;
        case '!':
            _operation = NOT;
            break;
        case '@':
            _operation = MEM_LOCATION;
            break;
        case '=':
            _operation = EQUAL;
            break;
        default:
            return false;
    }
    process_operation(_operation);
    return true;
}

/*
 * This parses through an operation and adds the operation to the intermediates.
 * Returns true if it read something.
 */
bool salmon_parse_operation(char* string)
{
    #define U16_CHAR(a,b) a | (b << 8)
    if (strlen(string) == 1 && salmon_parse_single_char_operation(*string))
        return true;
    intermediate_type _operation;
    switch (*((u16*)string)) {
        case U16_CHAR('<','<'):
            _operation = LSL;
            break;
        case U16_CHAR('>','>'):
            _operation = LSR;
            break;
        case U16_CHAR('=','='):
            _operation = IS_EQUAL;
            break;
        case U16_CHAR('<','='):
            _operation = GREATER_THAN_EQUAL;
            break;
        case U16_CHAR('+','+'):
            _operation = INC;
            break;
        case U16_CHAR('-','-'):
            _operation = DEC;
            break;
        case U16_CHAR('>','='):
            _operation = GREATER_THAN_EQUAL;
            break;
        default:
            return false;
    }
    process_operation(_operation);
    return true;
    #undef U16_CHAR
}