/*
 * This is the frontend for the Salmon programming language. TODO: This file 
 * should be named parser or something along those lines.
 */
// TODO: There should be a function to clear the out of scope variable names
// when we stop reading from a file.

#include<frontend/salmon/parser.h>
#include<frontend/common/parser.h>
#include<frontend/salmon/preprocessor.h>
#include<frontend/salmon/struct.h>
#include<frontend/salmon/types.h>
#include<intermediate/variable.h>
#include<intermediate/function.h>
#include<intermediate/intermediate.h>
#include<intermediate/struct.h>
#include<intermediate/enum.h>

/* This returns the front end of the inputted state ptr. */
#define GET_FRONT_END(state_ptr) (state_ptr->pass.data.front_end)

/* Invalid salmon variable, function, enum, etc. names. */
static const char* invalid_names[] = { "if", "fn", "let", "break", "return",
"loop", "" };

/* Salmon type names. */
static const char* type_names[] = { "abstract", "bool", "i8", "u8", "i16",
"u16", "i32", "u32", "i64", "u64", "f32", "f64", "%", "%", "" };

// TODO: These should be under the intermediate pass.
/* Salmon number suffixes. */
static const char number_suffixes[] = { 'u', 'U', 'l', 'L', 'f', 'F', 'd',
'D', '\0' };

/* Salmon number prefixes. */
static const char number_prefixes[] = { 'x', 'o', 'b', '\0' };

/* This returns the variable symbol table of the inputted state ptr. */
#define STATE_VARS(_state_ptr)  ((_state_ptr)->pass.variables)

/* This returns the function symbol table of the inputted state ptr. */
#define STATE_FUNCS(_state_ptr) ((_state_ptr)->pass.functions)

// TODO: This should return a derfrenced "char*" that can just be & liked
// "VECTOR_AT".
/*
 * This returns the string at the inputted index in the inputted state's source
 * file.
 */
#define STATE_AT(_state, _index) ((char**)vector_at(&(_state).source_file, \
_index, false))

// TODO: "_pass" should probally be named pass.
/* struct parser_state - This is the state of parsing in the Salmon source file
 * @file_names: A stack of string of the files being parsed for includes
 * @source_file: A vector of strings of the source file
 * @error_file: Used for error handling
 * @intermediate_pass: This intermediate pass
 */
typedef struct parser_state {
    stack file_names;
    vector source_file;
    vector error_file;
    intermediate_pass pass;
} parser_state;

// TODO: This needs to be cleaned.
static inline void salmon_parse_struct(parser_state* state, u32* location);
static inline void salmon_parse_loop(parser_state* state, u32* location);
static inline void salmon_parse_else(parser_state* state, u32* location);
static inline bool salmon_get_type(parser_state* state, u32* location);
static inline void salmon_parse_let(parser_state* state, u32* location);
static inline void salmon_parse_if(parser_state* state, u32* location);
static inline void salmon_parse_fn(parser_state* state, u32* location);
static inline bool salmon_parse_flow_operators(parser_state* state, \
char* string);
static inline bool salmon_parse_operation(parser_state* _state, char* string);
static inline void salmon_parse_enum(parser_state* state, u32* location);
static inline bool salmon_parse_enum_entry(parser_state* state, u32* location);
static inline void salmon_parse_typedef(parser_state* state, u32* location);
static inline bool salmon_parse_initial_syntax_tree(parser_state* state, \
u32* location);
static inline bool salmon_parse_struct_variable(parser_state* state, u32* \
location);

// TODO: Add the correction color
const char* error_range_type_colors[] = {"", "\x1b[0935m", "\x1b[091m"};

#if DESCRIPTIVE_ERRORS

// TODO: This is temperary and very badly done.
/* This returns UINT32_MAX if it wasn't found. */
u32 salmon_error_file_get_token_index(vector* _vec, char** token)
{
    for (u32 i=0; i < VECTOR_SIZE(*_vec); i++)
        if (&VECTOR_AT(_vec, i, char*) == token)
            return i;
    return UINT32_MAX;
}

#endif

/*
 * This function acts as the error handling function while parsing Salmon source
 * this reads errno and handles errors based on the value.
 */
void salmon_parser_handle_error(parser_state* state)
{
    // TODO: Reimplement this.
    return;

    #if DESCRIPTIVE_ERRORS
    /*
     * The ranges of the tokens the error occured on. The pre and post token
     * printing is done assuming the last link is the first error.
     */
    stack error_token_ranges = {
        .top = NULLPTR,
    };

    /* Values taken off of the error stack. */
    void* current_value;

    /* The current error token range. */
    error_token_range current_range;
    current_range.overide_token = NULLPTR;
    current_range.spaced = false;

    /* The type of the token range. */
    error_token_range current_range_type;

    /* The current token. */
    char** current_token;

    /* Used for type parsing errors. */
    char** type_names = GET_FRONT_END(state)->type_names;

    /* A u32 value used for a bunch of things. */
    u32 value;
    #endif

    /* This prints the inital error message and sets "tokens". */
    switch (errno)
    {
    case PARSING_ERROR_TYPE_PTRS_UNEQUAL:
        // TODO: These should still try and print the token that caused the
        // error.
        printf("%s Unequal ptr symbols before and after type\n", \
        ERROR_STRING);

        #if DESCRIPTIVE_ERRORS
        current_token = stack_pop(&error_value);
        current_range.type = ERROR_RANGE_TYPE_FAILED;
        current_range.starting_token = current_token;
        value = 0;

        /* Skipping the pre ptrs. */
        while (current_token[0][0] == type_names[0xd][0]) {
            current_token++;
            value++;
        }

        value++;
        current_token++;

        /* Skipping the post ptrs. */
        while (current_token[0][0] == type_names[0xd][0]) {
            current_token++;
            value--;
        }

        current_range.ending_token = current_token;
        STACK_PUSH_MALLOC(&error_token_ranges, &current_range);
        #endif

        break;
    case PARSING_ERROR_EXPECTED_TYPE:
        printf("%s Expected to find type\n", ERROR_STRING);

        #if DESCRIPTIVE_ERRORS
        current_token = stack_pop(&error_value);
        current_range.type = ERROR_RANGE_TYPE_FAILED;
        current_range.starting_token = current_token;
        current_range.ending_token = current_token + 1;
        STACK_PUSH_MALLOC(&error_token_ranges, &current_range);
        #endif

        break;
    case PARSING_ERROR_INVALID_VAR_NAME:

        #if DESCRIPTIVE_ERRORS
        current_token = stack_pop(&error_value);
        current_range = parser_handle_error(&state->pass,errno,current_token);
        current_range.starting_token = current_token;
        current_range.ending_token = current_token + 1;
        STACK_PUSH_MALLOC(&error_token_ranges, &current_range);
        #else
        printf("%s Invalid variable name\n", ERROR_STRING);
        #endif

        break;
    }

    #if DESCRIPTIVE_ERRORS

    /* Printing the pre tokens. */
    current_token = \
        ((error_token_range*)stack_last(&error_token_ranges))->starting_token;
    value = salmon_error_file_get_token_index(&state->source_file, \
    current_token);

    /* Scaling the value to make sure it doesn't go under 0. */
    value = value < SALMON_PARSING_ERROR_PRE_TOKENS ? value : \
    SALMON_PARSING_ERROR_PRE_TOKENS;

    current_token -= value;

    for (; *(i32*)&value > 0; value--, current_token++)
        if (*current_token != NULLPTR)
            printf("%s ", *current_token);

    /* Printing the errors. */
    while (!STACK_IS_EMPTY(error_token_ranges)) {
        current_range = *(error_token_range*)stack_pop(&error_token_ranges);

        if (current_range.overide_token != NULLPTR) {
            current_token = current_range.ending_token;
            printf("%s", current_range.overide_token);
            free(current_range.overide_token);
            continue;
        }

        if (error_range_type_colors[current_range.type][0] != '\0')
            printf("%s", error_range_type_colors[current_range.type]);

        for (current_token = current_range.starting_token; current_token < \
        current_range.ending_token; current_token++) {
            printf("%s", *current_token);
            if (current_range.spaced)
                printf(" ");
        }

        if (error_range_type_colors[current_range.type][0] != '\0')
            printf("\x1b[0m");
    }

    printf(" ");

    /* Printing the post tokens. */
    value = salmon_error_file_get_token_index(&state->source_file, \
    current_token);

    /* Scaling the value so it doesn't go further than the vector. */
    value = value + SALMON_PARSING_ERROR_PRE_TOKENS > \
    VECTOR_SIZE(state->source_file) - 1 ? VECTOR_SIZE(state->source_file) - 1 \
    - value : SALMON_PARSING_ERROR_PRE_TOKENS;

    for (; *(i32*)&value > 0 && current_token; value--, \
    current_token++) {
        if (*current_token != NULLPTR)
            printf("%s ", *current_token);
    }
    printf("\n");
    #endif


    exit_for_error();
}

/*
 * This function is used to init the data in an intermediate pass to the salmon
 * front end for things like variable names and tokenizer characters. This is
 * used when initing salmon parsing and when salmon parsing data is needed on an
 * intermediate pass that doesn't contain it.
 */
void salmon_parser_init_front_end(intermediate_pass* _pass)
{
    _pass->data.front_end = malloc(sizeof(front_end_pass));
    CHECK_MALLOC(_pass->data.front_end);
    memset(_pass->data.front_end, 0, sizeof(front_end_pass));
    _pass->data.front_end->type_names = (char**)type_names;
    _pass->data.front_end->invalid_names = (char**)invalid_names;
}

/*
 * This function turns a single salmon source file into intermediate tokens
 * and returns an "intermediate_pass".
 */
intermediate_pass salmon_file_into_intermediates(char* file_name)
{
    // TODO: This error system doesn't work with multithreading.
    error_handler = &salmon_parser_handle_error;

    tokenize_file_return tokenized_return = salmon_preprocess_file(file_name);

    /* Initing the parsing state. */
    parser_state state = {
        .pass = init_intermediate_pass(),
        .file_names =  {
            .top = NULLPTR
        },
        .source_file = tokenized_return.source_vector,
    };

    stack_push(&state.file_names, file_name);

    START_PROFILING("process file", "compile file");

    state.pass.variables = hash_table_init(8);
    state.pass.functions = hash_table_init(8);
    state.pass.enums = hash_table_init(4);
    state.pass.typedefs = hash_table_init(4);
    state.pass.structs = hash_table_init(4);

    salmon_parser_init_front_end(&state.pass);

    state.pass.data.front_end->white_space_chars = \
        (char*)tokenized_return.white_space_chars;
    state.pass.data.front_end->special_chars = \
        (char*)tokenized_return.special_chars;

    for (u32 i=0; i < VECTOR_SIZE(state.source_file); i++) {
        char* current_token = *STATE_AT(state, i);

        if (salmon_parse_initial_syntax_tree(&state, &i)
        || salmon_get_type(&state, &i)
        || salmon_parse_operation(&state, current_token)
        || salmon_parse_flow_operators(&state, current_token)
        || salmon_parse_struct_variable(&state, &i)
        || salmon_parse_enum_entry(&state, &i))
            continue;

        /* Checking if this is the end of a scope. */
        if (*current_token == '}') {
            intermediate _tmp_intermediate = {
                .type = END,
                .ptr = NULLPTR,
            };

            add_intermediate(&state.pass, _tmp_intermediate);
            continue;
        }

        /* Checking if this is a var access. */
        if (!is_invalid_name(&state.pass, current_token) \
        && get_variable_symbol_from_name(&state.pass, current_token) \
        != NULLPTR) {
            intermediate _tmp_intermediate = {
                .type = VAR_ACCESS,
                .ptr = get_variable_symbol_from_name(&state.pass,
                current_token),
            };

            add_operand(&state.pass, _tmp_intermediate, false);
        }

        /* Reading this as a possible float and incrementing to after it. */
        i += add_if_ascii_float(&state.pass, STATE_AT(state, i));

        // TODO: What does this comment mean?
        /* "vector_at" is used because "add_if_ascii_float" changes "i". */
        add_if_ascii_num (
            &state.pass,
            *STATE_AT(state, i),
            number_prefixes,
            number_suffixes
        );
    }
    
    END_PROFILING("process file", true);

    clear_operand_stack(&state.pass);
    free_tokenized_file_vector(&state.source_file);

    return state.pass;
}

// TODO: A lot of these functions don't check for the end of the file so they
// have a chance of seg faulting if we do reach the end of the file. 
/*
 * This function parses through the initial salmon syntax tree by reading the
 * initial keyword and calling the needed functions. This returns true if it
 * does read something.
 */
static inline bool salmon_parse_initial_syntax_tree(parser_state* state, \
u32* location)
{
    // TODO: This should hash the string ONCE then check the hash and all
    // functions that it calls should do the same there should be no real
    // strings except for reading constants. This would call for a major
    // redesign but it would lower memory consumption and increase speed.
    char* _token = *STATE_AT(*state, *location);
    if (!strcmp(_token, "if"))
        salmon_parse_if(state, location);
    else if (!strcmp(_token, "else"))
        salmon_parse_else(state, location);
    else if (!strcmp(_token, "fn"))
        salmon_parse_fn(state, location);
    else if (!strcmp(_token, "let"))
        salmon_parse_let(state, location);
    else if (!strcmp(_token, "loop"))
        salmon_parse_loop(state, location);
    else if (!strcmp(_token, "struct"))
        salmon_parse_struct(state, location);
    else if (!strcmp(_token, "enum"))
        salmon_parse_enum(state, location);
    else if (!strcmp(_token, "typedef"))
        salmon_parse_typedef(state, location);
    else
        return false;

    return true;
}

/*
 * These functions are all secondary branches on the syntax tree.
 */
inline void salmon_parse_typedef(parser_state* state, u32* location)
{
    /* Getting the typedef name. */
    find_next_valid_token(&state->source_file, location);
    char* typedef_name = *STATE_AT(*state, *location);

    /* Reading the type of the typedef. */
    find_next_valid_token(&state->source_file, location);
    type _type = get_type(&state->pass, &state->source_file, *location);
    HANDLE_ERROR();

    /* Creating the typedef. */
    add_typedef(&state->pass, typedef_name, _type);
}
static inline type salmon_parse_enum_derived_type(parser_state* state,
u32* location)
{
    type result_type = {
        .kind = NO_TYPE,
    };

    /* Checking if there is a derived type. */
    find_next_valid_token(&state->source_file, location);
    if (**STATE_AT(*state, *location) != '(')
        return result_type;

    find_next_valid_token(&state->source_file, location);
    result_type = get_type(&state->pass, &state->source_file, *location);

    find_next_valid_token(&state->source_file, location);
    if (**STATE_AT(*state, *location) != ')')
        send_error("Expected ')' after derived type in enum");

    return result_type;
}
inline void salmon_parse_enum(parser_state* state, u32* location)
{
    /* Getting the enum name. */
    find_next_valid_token(&state->source_file, location);
    char* enum_name = *STATE_AT(*state, *location);

    type _type = salmon_parse_enum_derived_type(state, location);
    HANDLE_ERROR();

    if (_type.kind == UINT8_MAX)
        send_error("Expected derived type for enum");

    if (!IS_TYPE_FLOAT_OR_DOUBLE(_type) && !IS_TYPE_INT(_type))
        send_error("Derived types for enums must be numerical");

    /* Creating the enum. */
    intermediate_typedef* _enum = add_typedef(&state->pass, enum_name, _type);

    // TODO: This type should change based on the largest and lowest types used
    // in the enum.
    /* If there is no type assume it's a u32. */
    if (_type.kind == UINT8_MAX) {
        _type.kind = U32_TYPE;
        _type.ptr_count = 0;
        _type.extra_data = 0;
        _type.flags = 0;
    }

    /* Skipping forward based on the type. */
    char** type_names = GET_FRONT_END(state)->type_names;
    u32 skip = _type.ptr_count;
    *location += skip << ((!!type_names[0xc][0])+(!!type_names[0xd][0]-1));

    find_next_valid_token(&state->source_file, location);

    salmon_parse_enum_no_type_label:

    /* Testing for the '{'. */
    if (**STATE_AT(*state, *location) != '{')
        send_error("Expected '{' at the start of a struct definition.");

    // TODO: This doesn't support float or doubles
    char* _name;
    num _value = {
        .magnitude = 0,
    };
    bool has_entires = false;
    /* Reading the entries. */
    while (true) {
        /* Getting the name of the entry. */
        find_next_valid_token(&state->source_file, location);
        _name = *STATE_AT(*state, *location);

        if (*_name == '}') {
            if (!has_entires)
                return;

            printf("\x1b[091mERROR:\x1b[0m Expected another entry in the enum after ',', but found: \"%s\"\n", _name);
            exit(-1);
        }

        // TODO: This should parse and solve the equation provided
        /* Getting the value if there is one. */
        find_next_valid_token(&state->source_file, location);
        if (is_ascii_number(*STATE_AT(*state, *location), number_suffixes,
        number_prefixes)) {
            _value = get_ascii_number(*STATE_AT(*state, *location));
            find_next_valid_token(&state->source_file, location);
            if (**STATE_AT(*state, *location) != '=') {
                printf("\x1b[091mERROR:\x1b[0m Expected '=' after enum entry value but found: \"%s\"\n", \
                    *STATE_AT(*state, *location));
                exit(-1);
            }
            find_next_valid_token(&state->source_file, location);
        } else {
            _value.magnitude + (_value.negative) ? -1 : 1;
        }

        /* Adding the entry. */
        add_enum_entry(&state->pass, _enum, _value, _name);

        has_entires = true;

        /* Making sure there's a comma and checking if the enum ended. */
        if (**STATE_AT(*state, *location) == ',') {
            continue;
        } else if (**STATE_AT(*state, *location) == '}') {
            return;
        } else {
            /* If there was no '}' send an error. */
            printf("\x1b[091mERROR:\x1b[0m Expected '}' to end struct declaration, but found: %s\n",
                *STATE_AT(*state, *location));
            exit(-1);
        }
    }
}
static inline void salmon_copy_derived_members(parser_state* state,
intermediate_struct* new_struct, u32 derived_hash)
{
    // TODO: This should check if the struct is forward defined.
    /* The data of the struct being created. */
    salmon_struct_data* new_data = (void*)new_struct
        + sizeof(intermediate_struct);

    /* The derived struct. */
    intermediate_struct* _struct = get_struct(&state->pass, derived_hash);
    salmon_struct_data* _data = (void*)_struct + sizeof(intermediate_struct);

    /* Copying the static members. */
    link* _link = _data->statics.top;
    while (_link != NULLPTR) {
        stack_push(&new_data->statics, _link->value);
        _link = _link->next;
    }

    /* Copying the non-static members. */
    _link = _struct->contents.top;
    while (_link != NULLPTR) {
        stack_push(&new_struct->contents, _link->value);
        _link = _link->next;
    }
}
static inline void salmon_parse_struct_derived(parser_state* state,
u32* location, intermediate_struct* _struct, salmon_struct_data* data)
{
    while (true) {
        /*
         * Breaking if the struct's scope started or if this is a forward
         * definition.
         */
        if (**STATE_AT(*state, *location) == '{'
        || **STATE_AT(*state, *location) == ';')
            break;

        if (**STATE_AT(*state, *location) != '(')
            send_error("Expected '(' before struct derived type");

        /* Reading the derived struct. */
        find_next_valid_token(&state->source_file, location);
        type _type = get_type(&state->pass, &state->source_file, *location);
        HANDLE_ERROR();

        if (_type.kind == UINT8_MAX)
            send_error("Unknown struct derived type");

        if (_type.kind != STRUCT_TYPE) {
            printf (
                "%s Structs can only be derived from other structs but found type: ",
                ERROR_STRING
            );

            print_type(&state->pass, _type);
            printf("\n");
            exit_for_error();
        }

        /* Adding the members from the derived struct. */
        salmon_copy_derived_members(state, _struct, _type.extra_data);

        /* Skipping past the type. */
        char** type_names = GET_FRONT_END(state)->type_names;
        u32 skip = _type.ptr_count;
        *location += skip << ((!!type_names[0xc][0])+(!!type_names[0xd][0]-1));

        find_next_valid_token(&state->source_file, location);
        if (**STATE_AT(*state, *location) != ')')
            send_error("Expected ')' after struct derived type");

        find_next_valid_token(&state->source_file, location);
    }
}
inline void salmon_parse_struct(parser_state* state, u32* location)
{
    /* Reading the struct name. */
    find_next_valid_token(&state->source_file, location);
    char** current_token = STATE_AT(*state, *location);
    intermediate_struct* _struct = create_struct (
        &state->pass,
        *current_token,
        sizeof(salmon_struct_data),
        0
    );

    /* Initing the extra salmon struct data. */
    salmon_struct_data* data = (void*)_struct + sizeof(intermediate_struct);
    data->statics.top = NULLPTR;

    find_next_valid_token(&state->source_file, location);
    salmon_parse_struct_derived(state, location, _struct, data);

    // TODO: This should support forward struct definition. Struct should have
    // a flag that says if it is foward defined.

    if (**STATE_AT(*state, *location) != '{')
        send_error("Expected '{' at the start of a struct definition.");


    /* Reading the variables in the struct. */
    type var_type;
    bool override;
    while (true) {
        /* Reading the name of the variable. */
        find_next_valid_token(&state->source_file, location);
        if (**STATE_AT(*state, *location) == '}')
            break;

        char* var_name = *STATE_AT(*state, *location);

        /* Checking if this is an override. */
        find_next_valid_token(&state->source_file, location);
        if (!strcmp(*STATE_AT(*state, *location), "override")) {
            override = true;
            find_next_valid_token(&state->source_file, location);
        } else {
            override = false;
        }

        /* Reading the type of the variable. */
        var_type = get_type(&state->pass, &state->source_file, *location);
        HANDLE_ERROR();
        if (var_type.kind == UINT8_MAX)
            send_error("Invalid type in struct variable");

        /* Skipping forward based on the type. */
        char** type_names = GET_FRONT_END(state)->type_names;
        u32 skip = var_type.ptr_count;
        *location += skip << ((!!type_names[0xc][0])+(!!type_names[0xd][0]-1));

        /*
         * If this type is invalid, this is assumed to be the end of the struct
         * definition.
         */
        if (var_type.kind == UINT8_MAX)
            break;

        // TODO: This should call a Salmon specific function to do this because
        // it needs to check for forward definitions.

        /* Adding the variable to the struct. */
        // TODO: Fix this
        if (var_type.flags & STATIC_FLAG) {
            salmon_add_static_member_to_struct (
                data,
                var_type,
                var_name,
                override
            );
        } else {
            add_variable_to_struct (
                &state->pass,
                _struct,
                var_type,
                var_name,
                override
            );
        }
    }

    if (**STATE_AT(*state, *location) != '}')
        send_error("Expected '}' at the end of a struct definition.");
}
inline void salmon_parse_if(parser_state* state, u32* location)
{
    intermediate _intermediate = {
        .type = IF,
        .ptr = NULLPTR,
    };

    add_intermediate(&state->pass, _intermediate);
}
inline void salmon_parse_else(parser_state* state, u32* location)
{
    intermediate _intermediate = {
        .type = ELSE,
        .ptr = NULLPTR,
    };

    add_intermediate(&state->pass, _intermediate);
}
inline void salmon_parse_fn(parser_state* state, u32* location)
{
    vector inputs = {
        .contents = NULLPTR,
        .apparent_size = 0,
        .size = 0,
        .type_size = sizeof(type),
    };

    clear_variables_in_scope(&state->pass);

    if (**(char**)STATE_AT(*state, (*location)+1) == '$')
        goto salmon_parse_fn_read_fn_name_label;

    *location += 1;

    /* This reads through the input variables. */
    for (; *location < VECTOR_SIZE(state->source_file); *location += 1) {
        char* _name = *STATE_AT(*state, *location);
        if (is_invalid_name(&state->pass, _name)) {
            printf(\
            "\x1b[091mERROR:\x1b[0m Invalid input variable name: \"%s\"\n", \
            _name);
            exit(-1);
        }

        *location += 1;

        type _type = get_type(&state->pass, &state->source_file, *location);
        HANDLE_ERROR();
        if (_type.kind == UINT8_MAX)
            send_error("Unknown type in function inputs");

        *location += 1;

        vector_append(&inputs, &_type);
        if (**STATE_AT(*state, *location) != ',')
            break;
    }

    if (**(u16**)STATE_AT(*state, *location) != ('<' | ('-' << 8)))
        send_error("Expected \"<-\" after function inputs");

    /* This reads the function name. */
    salmon_parse_fn_read_fn_name_label:
    *location += 1;
    if (**STATE_AT(*state, *location) != '$')
        send_error("Expected a '$' before function name");

    *location += 1;
    char* fn_name = *STATE_AT(*state, *location);
    if (is_invalid_name(&state->pass, fn_name))
        send_error("Function name is not valid");

    *location += 1;
    if (**STATE_AT(*state, *location) != '$')
        send_error("Expected a '$' after function name");

    /* This reads the function return type. */
    *location += 1;
    type return_type = {
        .kind = UINT8_MAX,
    };

    if (**(u16**)STATE_AT(*state, *location) == ('-' | ('>' << 8))) {
        return_type = get_type(&state->pass, &state->source_file, *location+1);
        HANDLE_ERROR();
    }

    if (!add_function_symbol(&state->pass, fn_name, inputs, return_type, 0))
        send_error("Function name already used");

    // for (; *location < VECTOR_SIZE((*file)); *location += 1)
        // if (**(char**)(vector_at(file, *location, false)) == '}')
            // break;

    *location += 2;

    clear_operand_stack(&state->pass);

    // TODO: This should put the function symbol into the ptr of the
    // intermediate.

    intermediate _intermediate = {
        .type = FUNC_DEF,
        .ptr = NULLPTR,
    };

    add_intermediate(&state->pass, _intermediate);

}
/* This sets errno on errors. */
inline void salmon_parse_let(parser_state* state, u32* location)
{
    /* Reading the variable name. */
    *location += 1;
    if (is_invalid_name(&state->pass, *STATE_AT(*state, *location))) {
        #if DESCRIPTIVE_ERRORS
        stack_push(&error_value, STATE_AT(*state, *location));
        #endif
        HANDLE_ERROR_WITH_CODE(PARSING_ERROR_INVALID_VAR_NAME);
    }
    char* name = *STATE_AT(*state, *location);

    /* Reading the variable type. */
    *location += 1;
    type _type = get_type(&state->pass, &state->source_file, *location);
    HANDLE_ERROR();
    if (_type.kind == UINT8_MAX) {
        #if DESCRIPTIVE_ERRORS
        stack_push(&error_value, STATE_AT(*state, *location));
        #endif
        HANDLE_ERROR_WITH_CODE(PARSING_ERROR_EXPECTED_TYPE);
    }

    /* Trying to add the variable to the symbol table. */
    variable_symbol* var = add_variable_symbol(&state->pass, name, _type, 0);
    if (var == NULLPTR) {
        printf("%s\n", name);
        #if DESCRIPTIVE_ERRORS
        stack_push(&error_value, STATE_AT(*state, *location));
        #endif
        HANDLE_ERROR_WITH_CODE(PARSING_ERROR_VAR_NAME_USED);
        exit(-1);
    }

    char** type_names = GET_FRONT_END(state)->type_names;

    /* 
     * This skips forward based on the number of before and after pointer
     * idicator chars.
     */
    u32 skip = _type.ptr_count;

    // TODO: These skipping things need to be updated a little.
    *location += skip << ((bool)type_names[0xc][0]+(bool)type_names[0xd][0]-1);

    /* Defining the decleration intermediate. */
    intermediate declaration = {
        .type = VAR_DECLARATION,
        .ptr = (void*)var,
    };

    add_intermediate(&state->pass, declaration);
    declaration.type = VAR_RETURN;
    declaration.ptr = (void*)var;
    add_operand(&state->pass, declaration, false);
}
inline void salmon_parse_loop(parser_state* state, u32* location)
{
    intermediate _intermediate = {
        .type = LOOP,
        .ptr = NULLPTR,
    };

    add_intermediate(&state->pass, _intermediate);
}

/* This parses a type and if found sets the top operand to that type. */
static inline bool salmon_get_type(parser_state* state, u32* location)
{
    if (*location >= VECTOR_SIZE(state->source_file))
        return false;

    type _type = get_type(&state->pass, &state->source_file, *location);
    HANDLE_ERROR();
    if (_type.kind == UINT8_MAX)
        return false;

    /* Skipping the types and ptr chars. */
    for (u32 i = 0; i++; i < (_type.ptr_count << 1))
        find_next_valid_token(&state->source_file, location);

    add_cast_intermediate(&state->pass, _type);

    return true;
}

/*
 * This parses through single character operations and adds the operations the
 * intermediates. Returns true if it read something.
 */
static inline bool salmon_parse_single_char_operation(parser_state* state, \
char _char)
{
    intermediate_type _operation;
    switch (_char)
    {
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
        _operation = MEM_DEREF;
        break;
    case '#':
        _operation = MEM_LOCATION;
        break;
    case '=':
        _operation = EQUAL;
        break;
    case ';':
        _operation = CLEAR_STACK;
        break;
	case '`':
		_operation = NEG;
		break;
    default:
        return false;
    }
    process_operation(&state->pass, _operation);
    return true;
}

/*
 * This parses through a flow operator and adds it to the intermediates. Return
 * true if reading was successful.
 */
static inline bool salmon_parse_flow_operators(parser_state* state, \
char* string)
{
    intermediate _tmp_intermediate = {
        .type = 0,
        .ptr = NULLPTR,
    };

    if (!strcmp(string, "break"))
        _tmp_intermediate.type = BREAK;
    else if (!strcmp(string, "continue"))
        _tmp_intermediate.type = CONTINUE;
    else
        return false;

    add_intermediate(&state->pass, _tmp_intermediate);
    return true;
}

/*
 * This parses the token at location and if the token is an enum entry it adds
 * it to the stack. Returns true if it read something.
 */
static inline bool salmon_parse_enum_entry(parser_state* state, u32* location)
{
    /* Getting the hash of the token. */
    HASH_STRING((*STATE_AT(*state, *location)));

    /* Getting the enum entry. */
    enum_entry* _entry = get_enum_entry(&state->pass.enums, result_hash);

    /* Checking if it's an enum entry. */
    if (_entry == NULLPTR)
        return false;

    /* Adding the entry to the stack. */
    add_const_num(&state->pass, _entry->value);

    /* Make sure there's something on the operand stack. */
    if (STACK_IS_EMPTY(state->pass.operand_stack))
        return false;

    /* Casting the top operand. */
    operand* top_operand = stack_top(&state->pass.operand_stack);
    top_operand->type = _entry->parent_enum->type;

    return true;
}

/*
 * This parses a struct variable and adds the operation to the intermediates.
 * Returns true if it read something.
 */
static inline bool salmon_parse_struct_variable(parser_state* state, \
u32* location)
{
    /*
     * If the size of operand stack is zero there are no operands to get a
     * variable from.
     */
    if (STACK_IS_EMPTY(state->pass.operand_stack))
        return false;

    /* Making sure the top operand is a struct and we are accessing a var. */
    operand* top_operand = stack_top(&state->pass.operand_stack);
    if (!(**STATE_AT(*state, *location) == '.' \
    && top_operand->type.kind == STRUCT_TYPE) || top_operand == NULLPTR)
        return false;

    /* Getting the struct variable's name. */
    find_next_valid_token(&state->source_file, location);
    char* variable_name = *STATE_AT(*state, *location);

    /* If the struct is a ptr it has to be dereference. */
    if (top_operand->type.ptr_count) {
        printf("\x1b[091mERROR:\x1b[0m Cannot get the member `%s` from a ", \
            variable_name);
        print_type_kind(&state->pass, top_operand->type);
        printf(": ");
        print_type(&state->pass, top_operand->type);
        printf("\n");
        exit(-1);
    }

    /* Getting the struct hash. */
    u32 struct_hash = top_operand->type.extra_data;

    /* Getting the struct variable. */
    struct_variable* _var = get_variable_from_struct_hash(&state->pass, \
    struct_hash, variable_name);

    /* Makes sure the variable is valid. */
    if (_var == NULLPTR)
        send_error("Unknown struct variable");

    /* Adding the intermediate. */
    intermediate struct_var_intermediate = {
        .type = GET_STRUCT_VARIABLE,
        .ptr = (void*)(size_t)_var->hash,
    };

    add_intermediate(&state->pass, struct_var_intermediate);

    /* Changing the type of the top operand to the struct variable's type. */
    top_operand->type = _var->type;

    return true;
}

/*
 * This parses through an operation and adds the operation to the intermediates.
 * Returns true if it read something.
 */
static inline bool salmon_parse_operation(parser_state* _state, char* string)
{
    #define U16_CHAR(a,b) a | (b << 8)

    if (strlen(string) == 1 \
    && salmon_parse_single_char_operation(_state, *string))
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
    process_operation(&_state->pass, _operation);
    return true;

    #undef U16_CHAR
}
