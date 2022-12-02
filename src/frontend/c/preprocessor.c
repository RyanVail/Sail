/*
 * This is the preprocessor for the Salmon programming language.
 */

#include<frontend/c/preprocessor.h>
#include<frontend/common/tokenizer.h>
#include<frontend/common/preprocessor.h>

static char white_space_c[] = { ' ', '\t' };
static char special_c[] = { '>', '<', '=', '!', '+', '-', '*', '/', '{', '}', \
';', '^', '!', '&', '|', ',', '(', ')', '$', '%', '#', '\\', '.', '\"', \
'\'', '~', ':', '[', ']', '?', '\n' };

static vector defined = { 0, 0, 0, sizeof(u32) };
static vector tokenized_file = { 0, 0, 0, sizeof(char*) };
static vector new_file = { NULL, 0, 0, sizeof(char*) };

static inline void C_expand_macro(vector* new_file, u32* i, vector* file);
static inline void C_read_macro(vector* file, u32* i);

/*
 * This preprocesses a single line of C code.
 */
void C_preprocess_line(u32* i)
{
    replace_C_const_chars(&tokenized_file, *i);
    replace_C_escape_codes(&tokenized_file, i);
    skip_C_comments(&tokenized_file, i);

    C_expand_macro(&new_file, i, &tokenized_file);
    C_read_macro(&tokenized_file, i);

    if (*i == VECTOR_SIZE(tokenized_file))
        return;

    vector_append(&new_file, vector_at(&tokenized_file, *i, false));
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

    for (u32 i=0; i < tokenized_file.apparent_size; i++) {
        if (*(char**)vector_at(&tokenized_file, i, false) == NULL)
            continue;
        C_preprocess_line(&i);
    }

    // for (u32 i=0; i < VECTOR_SIZE(new_file); i++)
        // printf("%s\n", *(char**)vector_at(&new_file, i, false));

    free(tokenized_file.contents);
    return new_file;
}

/*
 * This goes from the current position in the file till it reaches a '\n' and
 * returns the file index of the '\n'.
 */
static u32 C_get_end_of_line(vector* file, u32 i)
{
    for (; i < VECTOR_SIZE((*file)); i++)
        if (**(char**)vector_at(file, i, 0) == '\n')
            return i;

    return i;
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
    if (C_get_macro(file, *(char**)vector_at(file, *i, 0)) != 0)
        vector_append(&defined, i);
    *i = C_get_end_of_line(file, *i);
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

    u32 macro_end_index = C_get_end_of_line(file, macro_index) - 1;
    find_next_valid_token(file, i);

    if (*i == VECTOR_SIZE((*file)))
        return;

    char* current_token = 0;
    if (**(char**)vector_at(file, *i, 0) != '(') {
        for (; macro_index <= macro_end_index; macro_index++) {
            current_token = *(char**)vector_at(file, macro_index, 0);

            if (current_token == 0)
                continue;

            vector_append(new_file, &current_token);
        }
    }
}

/*
 * This is run during the C preprocessor loop and it skips single line comments.
 */
static inline void C_skip_single_line_comment(vector* file, u32* i)
{
    if (**(u16**)vector_at(file, *i, false) == '/' | ('/' << 8))
        *i = C_get_end_of_line(file, *i) + 1;
}

/*
 * This computes the result of an "#if" macro after all other macros inside of
 * return true or false based on the result.
 */
static inline void C_process_if_macro(vector* file, u32 *i)
{
    /*
     * I apologize for the abuse of macros but without it this section is even
     * more unreadable.
     */
    #define CURRENT_STRING *(char**)vector_at(file, *i, false)
    #define CURRENT_CHAR **(char**)vector_at(file, *i, false)
    #define GOTO_NEXT_LINE() *i = C_get_end_of_line(file, *i) + 1
    u32 _tmp_i = *i + 1;
    C_preprocess_line(&_tmp_i);
    // TODO: This should compute the result of the current line
    if (CURRENT_CHAR == '0') {
        GOTO_NEXT_LINE();
        for (; *i < VECTOR_SIZE((*file)); *i += 1) {
            if (CURRENT_CHAR == '#') {
                find_next_valid_token(file, i);
                if (!strcmp(CURRENT_STRING, "else")) {
                    GOTO_NEXT_LINE();
                    return;
                }
                GOTO_NEXT_LINE();
            }
        }
        send_error("Expected #endif after #if");
    } else {
        bool in_else = 0;
        GOTO_NEXT_LINE();
        for (; *i < VECTOR_SIZE((*file)); *i += 1) {
            if (CURRENT_CHAR == '#') {
                find_next_valid_token(file, i);
                if (!strcmp(CURRENT_STRING, "endif")) {
                    GOTO_NEXT_LINE();
                    return;
                }
                if (!strcmp(CURRENT_STRING, "else"))
                    in_else = 1;
                GOTO_NEXT_LINE();
            } else if (!in_else)
                C_preprocess_line(i);

        }
        send_error("Expected #endif after #if");
    }
    #undef CURRENT_STRING
    #undef CURRENT_CHAR
    #undef GOTO_NEXT_LINE
}

/*
 * This is run during the C preprocessor pass and it reads macros.
 */
static inline void C_read_macro(vector* file, u32* i)
{
    /*
    #if
    #ifdef
    #ifndef
    #else
    #endif
    #define
    #undef
    #include
    #error
    #pragma // pragmas are skipped
    */
    if (**(char**)vector_at(file, *i, false) != '#')
        return;

    find_next_valid_token(file, i);
    char* current_token = *(char**)vector_at(file, *i, false);
    find_next_valid_token(file, i);

    if (!strcmp(current_token, "if")) {
        C_process_if_macro(file, i);
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
    } else if (!strcmp(current_token, "error")) {
        //
    } else if (!strcmp(current_token, "pragma")
    || !strcmp(current_token, "endif")) {
        *i = C_get_end_of_line(file, *i) + 1;
    } else
        return;
}