/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/common/parser.h>
#include<frontend/c/preprocessor.h>
#include<evaluate.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>
#include<intermediate/intermediate.h>
#if DEBUG && linux
#include<cli.h>
#include<time.h>
#endif

static char white_space_c[] = { ' ', '\t' };
static char special_c[] = { '>', '<', '=', '!', '+', '-', '*', '/', '{', '}', \
';', '^', '!', '&', '|', ',', '(', ')', '$', '%', '#', '\\', '.', '\"', \
'\'', '~', ':', '[', ']', '?', '\n' };

/* The status given by macro functions. */
typedef enum macro_status {
    SAVE_LINE,          /* Add the current line to the output file. */
    SKIP_TO_NEXT,       /* Skip to the next #else, #elif, or #endif macro. */
    DONT_SAVE_LINE,     /* Don't add the current line to the output file. */
} macro_status;

static vector defined = { 0, 0, 0, sizeof(u32) };
// TODO: ^^^ This should be a hash table. ^^^
static vector tokenized_file = { 0, 0, 0, sizeof(char*) };
static vector new_file = { 0, 0, 0, sizeof(char*) };
static stack operand_stack = { NULLPTR, sizeof(i64) };

static inline void C_expand_macro(vector* new_file, u32* i, vector* file);
static inline macro_status C_read_macro(vector* file, u32* i);

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
    for (; *i <= end_line_index; find_next_valid_token(&tokenized_file, i))
        C_preprocess_token(i);

    /* Reads the macros in this line from the first index in this line. */
    bool dont_save = false;
    u32 index;
    switch(C_read_macro(&tokenized_file, &first_index))
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

/*
 * This takes in a file. This returns a tokenized and preprocessed version of
 * the file.
 */
vector C_preprocess_file(char* file_name)
{
    /* 
     * This is a vector of the defined macros which is a pointer to the file
     * location the macro came from.
     */

    set_tokenizer_chars(white_space_c, special_c);

    tokenized_file = tokenize_file(file_name);

    #if DEBUG && linux
    clock_t starting_time = clock();
    #endif

    for (u32 i=0; i < tokenized_file.apparent_size; i++) {
        if (*(char**)vector_at(&tokenized_file, i, false) == NULL)
            continue;
        C_preprocess_line(&i);
    }

    for (u32 i=0; i < VECTOR_SIZE(new_file); i++) {
        if (*(char**)vector_at(&new_file, i, false) != NULLPTR)
            printf("%s\n", *(char**)vector_at(&new_file, i, false));
    }

    free(tokenized_file.contents);
    #if DEBUG && linux
    if (get_global_cli_options()->time_compilation)
        printf("Took %f ms to preprocess file.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f);
    #endif
    return new_file;
}

/*
 * This returns the file index of a macro by the same name as the inputed
 * "char*" or "__UINT32_MAX__". The returned index is one more than where the
 * name is located.
 */
static inline u32 C_get_macro(vector* file, char* macro_name)
{
    for (u32 i=0; i < VECTOR_SIZE(defined); i++)
        if (!strcmp( \
        *(char**)vector_at(file,*(u32*)vector_at(&defined, i, 0),0),macro_name))
            return *(u32*)vector_at(&defined, i, 0) + 1;

    return __UINT32_MAX__;
}

/*
 * This adds the macro at the file location to the "defined" vector.
 */
static inline void C_add_macro(vector* file, u32* i)
{
    if (C_get_macro(file, *(char**)vector_at(file, *i, 0)) == __UINT32_MAX__)
        send_error("Macro has already been defined");
    vector_append(&defined, i);
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

    if (macro_index == __UINT32_MAX__)
        return;

    u32 macro_end_index = get_end_of_line(file, macro_index) - 1;
    find_next_valid_token(file, i);

    if (*i == VECTOR_SIZE((*file)))
        return;

    char* current_token = 0;
    if (**(char**)vector_at(file, *i, 0) != '(') {
        for (; macro_index <= macro_end_index; macro_index++) {
            current_token = *(char**)vector_at(file, macro_index, 0);

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
    if (**(u16**)vector_at(file, *i, false) == '/' | ('/' << 8)) {
        *i = get_end_of_line(file, *i);
        find_next_valid_token(file, i);
    }
}

/*
 * This is a helper function of "C_process_if_macro". This adds the numeral
 * value of the inputed ASCII number at the inputed index to the operand_stack.
 * Returns "__INTPTR_MAX__" if nothing was read.
 */
char** C_preprocessor_process_operand(char** token)
{
    /* Making sure this is a number. */
    if (!is_ascii_number(*token))
        return __INTPTR_MAX__;

    /* Allocating space for the number. */
    i64* num = malloc(sizeof(i64));
    if (num == NULLPTR)
        handle_error(0);
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

    printf("%u\n", _operator);
    *(i64*)operand_stack.top->value = \
        evaluate_expression(second_operator, first_operator, _operator);
}

/*
 * This computes the result of an "#if" macro after all other macros inside of
 * return true or false based on the result.
 */
static inline macro_status C_process_if_macro(vector* file, u32 *i)
{
    /* Getting the first index of the expression. */
    find_next_valid_token(file, i);
    char** index = vector_at(file, *i, false);

    // TODO: This doesn't account for the '?' and ':' operators.

    /* Processing the expression after the if statment. */
    C_parse_operation(&index, &C_preprocessor_process_operator, \
    &C_preprocessor_process_operand);

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
static inline macro_status C_read_macro(vector* file, u32* i)
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
    char* current_token = *(char**)vector_at(file, *i, false);
    // find_next_valid_token(file, i);

    if (!strcmp(current_token, "if")) {
        return C_process_if_macro(file, i);
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