/*
 * This is the front end for the Salmon programming language. TODO: This file 
 * should be named parser or something along those lines.
 */
// TODO: There should be a function to clear the out of scope variable names
// when we stop reading from a file.

#include<frontend/salmon/salmon.h>
#include<frontend/common/parser.h>
#include<frontend/salmon/preprocessor.h>

#include<backend/intermediate/symboltable.h>
#include<backend/intermediate/intermediate.h>

static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location);
static inline void salmon_parse_loop(vector* file, u32* location);
static inline void salmon_parse_else(vector* file, u32* location);
static inline bool salmon_parse_type(vector* file, u32* location);
static inline void salmon_parse_let(vector* file, u32* location);
static inline void salmon_parse_if(vector* file, u32* location);
static inline void salmon_parse_fn(vector* file, u32* location);
static inline bool salmon_parse_flow_operators(char* string);
static inline bool salmon_parse_operation(char* string);

// TODO: Remove this syntax tree when it isn't needed anymore.
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
void salmon_file_into_intermediate(char* file_name)
{
    vector file = salmon_preprocess_file(file_name);
    for (u32 i=0; i < file.apparent_size; i++) {
        char* current_token = *(char**)vector_at(&file, i, false);
        // printf("%s\n", current_token);
        if (salmon_parse_initial_syntax_tree(&file, &i)
        || salmon_parse_type(&file, &i)
        || salmon_parse_operation(current_token)
        || salmon_parse_flow_operators(current_token))
            continue;

        if (*current_token == '}') {
            intermediate _tmp_intermediate = { END, 0 };
            add_intermediate(_tmp_intermediate);
            continue;
        }

        if (!is_invalid_name(current_token)
        && get_variable_symbol(current_token, 0)) {
            intermediate _tmp_intermediate = \
            { VAR_ACCESS, (void*)get_variable_symbol(current_token, 0)->hash };

            add_operand(_tmp_intermediate, false);
        }

        if (is_ascii_number(current_token)) {
            i64 _const_num = get_ascii_number(current_token);
            void* const_num;
            /*
             * If we have an i64 larger than the size of a pointer we store the
             * value on the heap and make the ptr point to the value.
             */
            #if !VOID_PTR_64BIT
            intermediate _operand;
            if (_const_num < ~((i64)1 << ((sizeof(void*) << 3))-1) + 1
            || _const_num > ((i64)1 << ((sizeof(void*) << 3)-1))-1) {
                const_num = malloc(sizeof(i64));
                if (const_num == NULLPTR)
                    handle_error(0);
                memcpy(const_num, &_const_num, sizeof(i64));
                intermediate _operand = { CONST_PTR, const_num };
            } else {
                const_num = (void*)_const_num;
                intermediate _operand = { CONST, const_num };
            }
            #else
            const_num = (void*)_const_num;
            intermediate _operand = { CONST, const_num };
            #endif
            add_operand(_operand, false);
        }
    }
    
    clear_operand_stack();
    free_tokenized_file_vector(&file);
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
    else if (!strcmp(_token, "loop"))
        salmon_parse_loop(file, location);
    else
        return false;

    return true;
}

/*
 * These functions are all secondary branches on the syntax tree.
 */
inline void salmon_parse_if(vector* file, u32* location)
{
    intermediate _intermediate = { IF, 0 };
    add_intermediate(_intermediate);
}
inline void salmon_parse_else(vector* file, u32* location)
{
    intermediate _intermediate = { ELSE, 0 };
    add_intermediate(_intermediate);
}
inline void salmon_parse_fn(vector* file, u32* location)
{
    clear_variables_in_scope();

    if (**(char**)vector_at(file, (*location)+1, false) == '$')
        goto salmon_parse_fn_read_fn_name_label;

    *location += 1;

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
        send_error("Function name already used");

    // for (; *location < VECTOR_SIZE((*file)); *location += 1)
        // if (**(char**)(vector_at(file, *location, false)) == '}')
            // break;

    *location += 2;

    clear_operand_stack();

    intermediate _intermediate = { FUNC_DEF, 0 };
    add_intermediate(_intermediate);

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

    if (!add_variable_symbol(name, _type, 0))
        send_error("Variable name already used");

    char** type_names = get_type_names();

    /* 
     * This skips forward based on the number of before and after pointer
     * idicator chars.
     */
    *location += \
        _type.ptr << ((bool)type_names[0xc][0] + (bool)type_names[0xd][0] - 1);

    variable_symbol* new_var_symbol = get_variable_symbol(name, 0);

    intermediate _decleration = \
        { VAR_DECLERATION, new_var_symbol };
    
    add_intermediate(_decleration);
    _decleration.type = VAR_RETURN;
    _decleration.ptr = new_var_symbol->hash;
    add_operand(_decleration, false);
}
inline void salmon_parse_loop(vector* file, u32* location)
{
    intermediate _intermediate = { LOOP, 0 };
    add_intermediate(_intermediate);
}

/*
 * This parses a type and if found sets the top operand to that type.
 */
static inline bool salmon_parse_type(vector* file, u32* location)
{
    if (*location >= VECTOR_SIZE((*file)))
        return false;

    type _type = parse_type((char**)vector_at(file, *location, false));
    if (_type.kind == 255)
        return false;

    char** type_names = get_type_names();
    /* 
     * This skips forward based on the number of before and after pointer
     * idicator chars.
     */
    *location += \
        _type.ptr << ((bool)type_names[0xc][0] + (bool)type_names[0xd][0] - 1);

    cast_top_operand(_type);

    // TODO: This should be done in "intermediate.c".
    #if VOID_PTR_64BIT
    intermediate _tmp_intermediate = { CAST, *((void**)&_type) };
    #else
    type* _tmp_type = malloc(sizeof(type));
    if (_tmp_type == NULLPTR)
        handle_error(0);
    *_tmp_type = *(type*)&_type;
    intermediate _tmp_intermediate = { CAST, (void*)_tmp_type };
    #endif

    add_intermediate(_tmp_intermediate);

    return true;
}

/*
 * This parses through single character operations and adds the operations the
 * intermediates. Returns true if it read something.
 */
static inline bool salmon_parse_single_char_operation(char _char)
{
    intermediate_type _operation;
    // TODO: Switch statments need to be standerdized inside of this project.
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
        case ';':
            _operation = CLEAR_STACK;
            break;
        default:
            return false;
    }
    process_operation(_operation);
    return true;
}

/*
 * This parses through a flow operator and adds it to the intermediates. Return
 * true if reading was successful.
 */
static inline bool salmon_parse_flow_operators(char* string)
{
    intermediate _tmp_intermediate = { 0, 0 };
    if (!strcmp(string, "break"))
        _tmp_intermediate.type = BREAK;
    else if (!strcmp(string, "continue"))
        _tmp_intermediate.type = CONTINUE;
    // else if (!strcmp(string, "goto"))
    else
        return false;

    add_intermediate(_tmp_intermediate);
    return true;
}

/*
 * This parses through an operation and adds the operation to the intermediates.
 * Returns true if it read something.
 */
static inline bool salmon_parse_operation(char* string)
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