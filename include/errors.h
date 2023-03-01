#ifndef ERRORS_H
#define ERRORS_H

#include<common.h>
#include<datastructures/stack.h>
#include<datastructures/vector.h>

/*
 * This is a ptr to the current error handling function. This function is
 * responsible for clearing the "error_value" stack after printing the
 * error / warning and errno back to 0 on soft errors and warnings.
 */
extern void (*error_handler)(void);

#if DESCRIPTIVE_ERRORS
/*
 * The place where the last error occurred. This might be pointing to an
 * intermediate token or other types of values which has to be determined based
 * on the current phase of compilation and the error type.
 */
extern stack error_value;
#endif

/*
 * This calls the function in "error_handler" to handle the current errno unless
 * it's a NULLPTR in which case it isn't called and the error value is cleared.
 * If there is no error this will not do anything.
 */
#define HANDLE_ERROR() \
    if (errno != 0) { \
        if (error_handler != NULLPTR) { \
            error_handler(); \
        } else { \
            clear_error_value_stack(); \
        } \
    }

/* This expands to "HANDLE_ERORR" and setting errno to the inputted code. */
#define HANDLE_ERROR_WITH_CODE(_code) \
    errno = _code; \
    HANDLE_ERROR()

// TODO: There should be a seperate macro for the error color.
#define ERROR_STRING "\x1b[091mERROR:\x1b[0m"
#define ERROR_COLOR_START "\x1b[091m"
#define COLOR_END "\x1b[0m"

/* This clears the "error_value" stack. */
void clear_error_value_stack();

#if CHECK_MALLOC_RETURNS
/*
 * This handles a lot of common errors that don't need any extra processing
 * like failing to allocate memory. This should not be called directly, rather
 * through the "HANDLE_COMMON_ERROR" macro.
 */
void handle_common_error(u32 error_code);

#define HANDLE_COMMON_ERROR(error_code) handle_common_error(error_code)
#else
#define HANDLE_COMMON_ERROR(error_code)
#endif

/* This takes in a pointer and calls "HANDLE_COMMON_ERROR(0)" if it's a "NULLPTR". */
#define CHECK_MALLOC(x) \
    if ((x) == NULLPTR) { \
        HANDLE_COMMON_ERROR(0); \
    }

/* This handles sending custom error messages. */
void send_error(char* error_message);

/*
 * These are the error codes that errno can be set to during parsing. The value
 * in error_ptr is show in the comment next to the enum entry.
 */
typedef enum parsing_error {
    PARSING_ERORR_SUCCESS,              /* No values. */
    PARSING_ERROR_TYPE_PTRS_UNEQUAL,    /* The only ptr on the stack is the
                                        first token of the type. */
    PARSING_ERROR_EXPECTED_TYPE,        /* The only on the stack is the expected
                                        type token. */ 
    PARSING_ERROR_REACHED_BOT,          /* No values. */
    PARSING_ERROR_INVALID_VAR_NAME,     /* The name token is the only value. */
    PARSING_ERROR_VAR_NAME_USED,        /* The name token is the only value. */
} parsing_error;

#if CHECK_MALLOC_RETURNS
/*
 * This reads errno and based on the value it will backtrace the parsing to the
 * points that caused the error.
 */
void backtrace_parsing_error(vector* file, u32 index);
#endif

#endif