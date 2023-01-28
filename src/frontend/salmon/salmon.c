/*
 * This is the front end for the Salmon programming language. TODO: This file 
 * should be named parser or something along those lines.
 */
// TODO: There should be a function to clear the out of scope variable names
// when we stop reading from a file.
// TODO: This file should be named "parser.c" to keep consistency.

#include<frontend/salmon/salmon.h>
#include<frontend/common/parser.h>
#include<frontend/salmon/preprocessor.h>
#include<intermediate/symboltable.h>
#include<intermediate/intermediate.h>
#include<intermediate/struct.h>
#include<intermediate/enum.h>

// TODO: This needs to be cleaned.

static inline bool salmon_parse_initial_syntax_tree(vector* file, u32* location);
static inline void salmon_parse_struct(vector* file, u32* location);
static inline void salmon_parse_loop(vector* file, u32* location);
static inline void salmon_parse_else(vector* file, u32* location);
static inline bool salmon_get_type(vector* file, u32* location);
static inline void salmon_parse_let(vector* file, u32* location);
static inline void salmon_parse_if(vector* file, u32* location);
static inline void salmon_parse_fn(vector* file, u32* location);
static inline bool salmon_parse_flow_operators(char* string);
static inline bool salmon_parse_operation(char* string);
static inline bool salmon_parse_struct_variable(vector* file, u32* location);
static inline void salmon_parse_enum(vector* file, u32* location);
static inline bool salmon_parse_enum_entry(vector* file, u32* location);

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
        || salmon_get_type(&file, &i)
        || salmon_parse_operation(current_token)
        || salmon_parse_flow_operators(current_token)
        || salmon_parse_struct_variable(&file, &i)
        || salmon_parse_enum_entry(&file, &i))
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

        add_if_ascii_num(current_token);
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
    // TODO: This should hash the string ONCE then check the hash and all
    // functions that it calls should do the same there should be no real
    // strings except for reading constants. This would call for a major
    // redesign but it would lower memory consumption and increase speed.
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
    else if (!strcmp(_token, "struct"))
        salmon_parse_struct(file, location);
    else if (!strcmp(_token, "enum"))
        salmon_parse_enum(file, location);
    else if (!strcmp(_token, "typedef"))
        salmon_parse_typedef(file, location);
    else
        return false;

    return true;
}

/*
 * These functions are all secondary branches on the syntax tree.
 */
inline void salmon_parse_typedef(vector* file, u32* location)
{
    /* Getting the typedef name. */
    find_next_valid_token(file, location);
    char* typedef_name = *(char**)vector_at(file, *location, false);

    /* Reading the type of the typedef. */
    find_next_valid_token(file, location);
    type _type = get_type((char**)vector_at(file, *location, false));

    /* Creating the typedef. */
    add_typedef(typedef_name, _type);
}
inline void salmon_parse_enum(vector* file, u32* location)
{
    /* Getting the enum name. */
    find_next_valid_token(file, location);
    char* enum_name = *(char**)vector_at(file, *location, false);

    /* Reading the type of the enum, if there is one. */
    find_next_valid_token(file, location);
    type _type = get_type((char**)vector_at(file, *location, false));

    /* Creating the enum. */
    intermediate_typedef* _enum = add_typedef(enum_name, _type);

    // TODO: This type should change based on the largest and lowest types used
    // in the enum.
    /* If there is no type assume it's a U32. */
    if (_type.kind == 255) {
        _type.kind = U32_TYPE;
        _type.ptr = NULLPTR;
        goto salmon_parse_enum_no_type_label;
    }

    /* Skipping forward based on the type. */
    char** type_names = get_type_names();
    u32 skip = IS_TYPE_STRUCT(_type) ? _type.kind >> 16 : _type.ptr;
    *location += skip << ((!!type_names[0xc][0])+(!!type_names[0xd][0]-1));

    find_next_valid_token(file, location);

    salmon_parse_enum_no_type_label:

    /* Testing for the '{'. */
    if (**(char**)vector_at(file, *location, false) != '{')
        send_error("Expected '{' at the start of a struct definition.");

    /* This is the counter for the entries in the enum. */
    u64 counter = 0;

    char* _name;
    i64 _value;
    char ending_char;
    /* Reading the entries. */
    while (true) {
        /* Getting the name of the entry. */
        find_next_valid_token(file, location);
        _name = *(char**)vector_at(file, *location, false);

        if (*_name == '}') {
            printf("\x1b[091mERROR:\x1b[0m Expected another entry in the enum after ',', but found: \"%s\"\n", _name);
            exit(-1);
        }

        // TODO: This should parse and solve the equation provided
        /* Getting the value if there is one. */
        find_next_valid_token(file, location);
        if (is_ascii_number(*(char**)vector_at(file, *location, false))) {
            _value = get_ascii_number(*(char**)vector_at(file, *location, 0));
            counter = _value++;
            find_next_valid_token(file, location);
            if (**(char**)vector_at(file, *location, false) != '=') {
                printf("\x1b[091mERROR:\x1b[0m Expected '=' after enum entry value but found: \"%s\"\n", \
                    *(char**)vector_at(file, *location, false));
                exit(-1);
            }
            find_next_valid_token(file, location);
        } else {
            _value = counter;
            counter++;
        }

        /* Adding the entry. */
        add_enum_entry(_enum, _value, _name);

        /* Making sure there's a comma and checking if the enum ended. */
        if (**(char**)vector_at(file, *location, false) == ',') {
            continue;
        } else if (**(char**)vector_at(file, *location, false) == '}') {
            return;
        } else {
            /* If there was no '}' send an error. */
            printf("\x1b[091mERROR:\x1b[0m Expected '}' to end struct decleration, but found: %s\n", \
                *(char**)vector_at(file, *location, false));
            exit(-1);
        }
    }
}
inline void salmon_parse_struct(vector* file, u32* location)
{
    /* Reading the struct name. */
    find_next_valid_token(file, location);
    char** current_token = (char**)vector_at(file, *location, false);
    intermediate_struct* _struct = create_struct(0, *current_token);

    // TODO: This should support forward struct definition. Struct should have
    // a flag that says if it is foward defined.

    find_next_valid_token(file, location);
    if (**(char**)vector_at(file, *location, false) != '{')
        send_error("Expected '}' at the start of a struct definition.");


    /* Reading the variables in the struct. */
    type var_type;
    while (true) {
        /* Getting the name of the variable. */
        find_next_valid_token(file, location);

        if (**(char**)vector_at(file, *location, false) == '}')
            break;

        char* var_name = *(char**)vector_at(file, *location, false);

        /* Getting the type of the variable. */
        find_next_valid_token(file, location);
        var_type = get_type((char**)vector_at(file, *location, false));

        /* Skipping forward based on the type. */
        char** type_names = get_type_names();
        if (var_type.kind == 255)
            send_error("Invalid type in struct variable");
        u32 skip = IS_TYPE_STRUCT(var_type) ? var_type.kind>>16 : var_type.ptr;
        *location += skip << ((!!type_names[0xc][0])+(!!type_names[0xd][0]-1));

        /*
         * If this type is invalid, this is assumed to be the end of the struct
         * definition.
         */
        if (var_type.kind == 255)
            break;

        /* Adding the variable to the struct. */
        add_variable_to_struct(_struct,var_type,var_name);
    }

    // reverse_struct_variables(_struct);
    // ARMv7_generate_struct(_struct);
    // reverse_struct_variables(_struct);
    // struct_variable* __var;
    // while (stack_size(&_struct->contents) != 0) {
    //     __var = stack_pop(&_struct->contents);
    //     printf("===================\nHASH: %08x\n", __var->hash);
    //     if (__var->hash == 0) {
    //         printf("PADDING: %u\n", (u32)__var->name);
    //     } else {
    //         printf("NAME: %s\n", __var->name);
    //     }
    // }
    // printf("===================\n");
    // exit(0);

    if (**(char**)vector_at(file, *location, false) != '}')
        send_error("Expected '}' at the end of a struct definition.");
}
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
    vector inputs = { 0, 0, 0, sizeof(type) };
    clear_variables_in_scope();

    if (**(char**)vector_at(file, (*location)+1, false) == '$')
        goto salmon_parse_fn_read_fn_name_label;

    *location += 1;

    /* This reads through the input variables. */
    for (; *location < VECTOR_SIZE((*file)); *location += 1) {
        char* _name = *(char**)vector_at(file, *location, false);
        if (is_invalid_name(_name))
            send_error("Invalid input variable name");

        *location += 1;

        type _type = get_type((char**)vector_at(file, *location, false));
        if (_type.kind == 255)
            send_error("Unknown type in function inputs");

        *location += 1;

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
        return_type = get_type((char**)vector_at(file, *location+1, false));

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
    type _type = get_type((char**)vector_at(file, *location, false));
    if (_type.kind == 255)
        send_error("Expected type after let");

    if (!add_variable_symbol(name, _type, 0))
        send_error("Variable name already used");

    char** type_names = get_type_names();

    /* 
     * This skips forward based on the number of before and after pointer
     * idicator chars.
     */
    u32 skip = IS_TYPE_STRUCT(_type) ? _type.kind >> 16 : _type.ptr;

    *location += \
        skip << ((bool)type_names[0xc][0] + (bool)type_names[0xd][0] - 1);

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
static inline bool salmon_get_type(vector* file, u32* location)
{
    if (*location >= VECTOR_SIZE((*file)))
        return false;

    type _type = get_type((char**)vector_at(file, *location, false));
    if (_type.kind == 255)
        return false;

    char** type_names = get_type_names();
    /* 
     * This skips forward based on the number of before and after pointer
     * idicator chars.
     */
    u32 skip = IS_TYPE_STRUCT(_type) ? _type.kind >> 16 : _type.ptr;
    *location += skip << ((bool)type_names[0xc][0]+(bool)type_names[0xd][0]-1);

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
        case '#':
            _operation = MEM_ACCESS;
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
 * This parses the token at location and if the token is an enum entry it adds
 * it to the stack. Returns true if it read something.
 */
static inline bool salmon_parse_enum_entry(vector* file, u32* location)
{
    /* Getting the hash of the token. */
    HASH_STRING((*(char**)vector_at(file, *location, false)));

    /* Getting the enum entry. */
    enum_entry* _entry = get_enum_entry(result_hash);

    /* Checking if it's an enum entry. */
    if (_entry == NULLPTR)
        return false;

    /* Adding the entry to the stack. */
    add_const_num(_entry->value);


    /* Make sure there's something on the operand stack. */
    stack* operand_stack = get_operand_stack();
    if (stack_size(operand_stack) == 0) {
        return false;
    }

    /* Casting the top operand. */
    operand* top_operand = stack_top(operand_stack);
    top_operand->type = _entry->parent_enum->type;

    return true;
}

/*
 * This parses a struct variable and adds the operation to the intermediates.
 * Returns true if it read something.
 */
static inline bool salmon_parse_struct_variable(vector* file, u32* location)
{
    /* Getting the operand stack. */
    stack* operand_stack = get_operand_stack();

    /*
     * If the size of operand stack is zero there are no operands to get a
     * variable from.
     */
    if (stack_size(operand_stack) == 0)
        return false;

    /* Making sure the top operand is a struct and we are accessing a var. */
    operand* top_operand = stack_top(operand_stack);
    if (top_operand == NULLPTR || !(**(char**)vector_at(file,*location,0) == '.'
    && IS_TYPE_STRUCT(top_operand->type)))
        return false;

    /* Getting the struct variable's name. */
    find_next_valid_token(file, location);
    char* variable_name = *(char**)vector_at(file, *location, false);

    /* If the struct is a ptr it has to be derfrenced. */
    if (top_operand->type.kind >> 16) {
        printf("\x1b[091mERROR:\x1b[0m Cannot get the member `%s` from a ", \
            variable_name);
        print_type_kind(top_operand->type, true);
        printf(": ");
        print_type(top_operand->type, true);
        printf("\n");
        exit(-1);
    }

    /* Getting the struct hash. */
    u32 struct_hash = top_operand->type.ptr;

    /* Getting the struct variable. */
    struct_variable* _var = \
        get_variable_from_struct(struct_hash, variable_name);

    /* Makes sure the variable is valid. */
    if (_var == NULLPTR)
        send_error("Uknown struct variable");

    /* Adding the intermediate. */
    intermediate struct_var_intermediate = { GET_STRUCT_VARIABLE, _var->hash };
    add_intermediate(struct_var_intermediate);

    /* Changing the type of the top operand to the struct variable's type. */
    top_operand->type = _var->type;

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