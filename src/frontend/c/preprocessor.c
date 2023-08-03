/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/common/parser.h>
#include<frontend/c/preprocessor.h>
#include<evaluate.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>
#include<intermediate/intermediate.h>

static char white_space_c[] = { ' ', '\t' };
static char special_c[] = { '>', '<', '=', '!', '+', '-', '*', '/', '{', '}',
';', '^', '!', '&', '|', ',', '(', ')', '$', '%', '#', '\\', '.', '\"', '\'',
'~', ':', '[', ']', '?', '\n' };

/* The status given by macro functions. */
typedef enum macro_status {
    SAVE_LINE,          /* Add the current line to the output file. */
    SKIP_TO_NEXT,       /* Skip to the next #else, #elif, or #endif macro. */
    DONT_SAVE_LINE,     /* Don't add the current line to the output file. */
} macro_status;

static vector defined = {
    .contents = NULLPTR,
    .apparent_size = 0,
    .size = 0,
    .type_size = sizeof(u32),
};
// TODO: ^^^ This should be a hash table. ^^^

static vector tokenized_file = {
    .contents = NULLPTR,
    .apparent_size = 0
    .size = 0,
    .type_size = sizeof(char*),
};

static vector new_file = {
    .contents = NULLPTR,
    .apparent_size = 0,
    .size = 0,
    .type_size = sizeof(char*),
};

static stack operand_stack = {
    .top = NULLPTR
};

static inline void C_expand_macro(vector* new_file, u32* i, vector* file);
static inline macro_status C_read_macro(vector* file, u32* i, \
char** ending_token);

// TODO: tokenized file is static now but it is still being passed into
// functions.

#include<frontend/c/preprocessor.h>
#include<frontend/c/parser.h>

/*
 * This preprocesses a single token of C code.
 */
void C_preprocess_token(u32* i)
{
    replace_C_const_chars(&tokenized_file, *i);
    // replace_C_escape_codes(&tokenized_file, i);

    // TODO: This needs more work because of "C_preprocess_line"
    // skip_C_comments(&tokenized_file, i);

    C_expand_macro(&new_file, i, &tokenized_file);

    if (*i >= VECTOR_SIZE(tokenized_file))
        return;

    vector_append(&new_file, vector_at(&tokenized_file, *i, false));
}

/*
 * This preprocesses a single line of C code.
 */
void C_preprocess_line(u32* i)
{
    u32 end_line_index = get_end_of_line(&tokenized_file, *i);

    /* Goes to the "\n" if we aren't at the start of the file. */
    *i = *i > 0 ? *i - 1 : *i;
    u32 first_index = *i;

    /* Preprocess the tokens in this line. */
    for (; *i < end_line_index; find_next_valid_token(&tokenized_file, i))
        C_preprocess_token(i);

    /* Reads the macros in this line from the first index in this line. */
    bool dont_save = false;
    u32 index;
    switch(C_read_macro(&tokenized_file, &first_index, \
    vector_at(&tokenized_file, *i, false)))
    {
    case DONT_SAVE_LINE:
        dont_save = true;
    case SKIP_TO_NEXT:
        index = new_file.apparent_size - 1;
        do {
            index--;
            if (*(char**)vector_at(&new_file,index,true) == NULLPTR)
                continue;
            if (**(char**)vector_at(&new_file,index,true) == '\n')
                break;
            free(*(char**)vector_at(&new_file,index,true));
            *(char**)vector_at(&new_file,index,true) = NULLPTR;
            new_file.apparent_size--;
        } while (index > 0);

        if (dont_save)
            return;

        for (; *i <= VECTOR_SIZE(tokenized_file) - 1; ) {
            if (*(char**)vector_at(&tokenized_file, *i, false) != NULLPTR
            && (!strcmp(*(char**)vector_at(&tokenized_file, *i, 0), "endif")
            || !strcmp(*(char**)vector_at(&tokenized_file, *i, 0), "else")))
                break;
            find_next_valid_token(&tokenized_file, i);
        }
        find_next_valid_token(&tokenized_file, i);
        find_next_valid_token(&tokenized_file, i);
        break;
    case SAVE_LINE:
        break;
    }
}

/* This takes in a file name and returns a preprocessed version of it. */
vector C_preprocess_file(char* file_name)
{
    set_tokenizer_chars(white_space_c, special_c);

    tokenized_file = tokenize_file(file_name).token_vector;

    START_PROFILING("preprocess file", "compile file");

    for (u32 i=0; i < tokenized_file.apparent_size; i++)
        if (VECTOR_AT(&tokenized_file, i, char*) != NULLPTR)
            C_preprocess_line(&i);

    for (u32 i=0; i < VECTOR_SIZE(new_file); i++)
        if (VECTOR_AT(&new_file, i, char*) != NULLPTR)
            printf("%s\n", VECTOR_AT(&new_file, i, char*));

    free(tokenized_file.contents);

    END_PROFILING("preprocess file", true);

    return new_file;
}

// TODO: This really should be done in a better way.
/*
 * This returns the file index of a macro by the same name as the inputted
 * "char*" or UINT32_MAX. The returned index is one more than where the
 * name is located.
 */
static inline u32 C_get_macro(vector* file, char* macro_name)
{
    for (u32 i=0; i < VECTOR_SIZE(defined); i++)
        if (!strcmp(VECTOR_AT(file, VECTOR_AT(&defined, i, u32), char*),
        macro_name))
            return VECTOR_AT(&defined, i, u32) + 1;

    return UINT32_MAX;
}

/*
 * This adds the macro at the file location to the "defined" vector. This should
 * be called with the i index of file pointer to the "define" token in the
 * "#define" macro.
 */
static inline void C_add_macro(vector* file, u32* i)
{
    /* Incramenting passed the define. */
    find_next_valid_token(file, i);

    /* Making sure the macro isn't already defined. */
    if (C_get_macro(file, *(char**)vector_at(file, *i, 0)) != UINT32_MAX)
        send_error("Macro has already been defined");

    /* Adding the macro to the defined vector. */
    vector_append(&defined, i);

    /* Getting the line after the macro. */
    *i = get_end_of_line(file, *i);
}

// TODO: This shouldn't be redoing this computation every time it expands a
// macro.
/*
 * This is run during the C preprocessor and it expands macros based on the
 * macro name.
 */
static inline void C_expand_macro(vector* new_file, u32* i, vector* file)
{
    char* name = *(char**)vector_at(file, *i, 0);
    u32 macro_index = C_get_macro(file, name);

    if (macro_index == UINT32_MAX)
        return;

    u32 macro_end_index = get_end_of_line(file, macro_index) - 1;
    find_next_valid_token(file, i);

    if (*i == VECTOR_SIZE(*file))
        return;

    char* current_token = 0;
    if (*VECTOR_AT(file, *i, char*) != '(') {
        for (; macro_index <= macro_end_index; macro_index++) {
            current_token = VECTOR_AT(file, macro_index, char*)

            if (current_token == NULLPTR)
                continue;

            vector_append(new_file, &current_token);
        }
    }
    find_next_valid_token(file, i);
}

/*
 * This is run during the C preprocessor loop and it skips single line comments.
 */
static inline void C_skip_single_line_comment(vector* file, u32* i)
{
    if (*VECTOR_AT(file, *i, u16*) == ('/' | ('/' << 8))) {
        *i = get_end_of_line(file, *i);
        find_next_valid_token(file, i);
    }
}

/*
 * This is a helper function of "C_process_if_macro". This adds the numeral
 * value of the inputted ASCII number at the inputted index to the
 * operand_stack. Returns INTPTR_MAX if nothing was read.
 */
char** C_preprocessor_process_operand(char** token)
{
    /* Making sure this is a number. */
    if (!is_ascii_number(*token))
        return (void*)INTPTR_MAX;

    /* Allocating space for the number. */
    i64* num = malloc(sizeof(i64));
    CHECK_MALLOC(num);
    *num = get_ascii_number(*token);

    /* Adding the num to the stack and returning the current token. */
    stack_push(&operand_stack, num);
    return token;
}

/*
 * This takes in and computes the result of the operation.
 */
void C_preprocessor_process_operator(operator _operator)
{
    i64 first_operator = *(i64*)stack_pop(&operand_stack);
    i64 second_operator = *(i64*)stack_top(&operand_stack);

    *(i64*)operand_stack.top->value = \
        evaluate_expression(second_operator, first_operator, _operator);
}

/*
 * This computes the result of an "#if" macro after all other macros inside of
 * return true or false based on the result.
 */
static inline macro_status C_process_if_macro(vector* file, u32 *i, \
char** ending_token)
{
    /* Getting the first index of the expression. */
    find_next_valid_token(file, i);
    char** index = &VECTOR_AT(file, *i, char*);

    // TODO: This doesn't account for the '?' and ':' operators.

    /* Processing the expression after the if statement. */
    C_parse_operation(&index, &C_preprocessor_process_operator, \
    &C_preprocessor_process_operand, ending_token);

    find_next_valid_token(file, i);

    if (stack_size(&operand_stack) != 1)
        send_error("Expected the result of preprocessing to be one number");

    i64 result = *(i64*)stack_top(&operand_stack);

    while (!STACK_IS_EMPTY(operand_stack))
        free(stack_pop(&operand_stack));

    if (result >= 1)
        return DONT_SAVE_LINE;

    return SKIP_TO_NEXT;
}

/*
 * This is run during the C preprocessor pass and it reads macros. This should
 * be pointed to the token right before a new line token.
 */
static inline macro_status C_read_macro(vector* file, u32* i, \
char** ending_token)
{
    /*
    #if
    #ifdef
    #ifndef
    #else
    #elif
    #endif
    #define
    #undef
    #include
    #error
    #line
    #pragma
    */

    if (**(char**)vector_at(file, *i, false) != '#')
        return SAVE_LINE;

    find_next_valid_token(file, i);
    char* current_token = VECTOR_AT(file, *i, char*);

    if (!strcmp(current_token, "if")) {
        return C_process_if_macro(file, i, ending_token);
    } else if (!strcmp(current_token, "ifdef")) {
        //
    } else if (!strcmp(current_token, "ifndef")) {
        //
    } else if (!strcmp(current_token, "define")) {
        C_add_macro(file, i);
    } else if (!strcmp(current_token, "undef")) {
        //
    } else if (!strcmp(current_token, "include")) {
        //
    } else if (!strcmp(current_token, "else")) {
        return SKIP_TO_NEXT;
    } else if (!strcmp(current_token, "error")) {
        //
    } else if (!strcmp(current_token, "pragma")
    || !strcmp(current_token, "endif")) {
        return DONT_SAVE_LINE;
    } else
        return DONT_SAVE_LINE;

    return DONT_SAVE_LINE;
}