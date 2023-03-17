#include<errors.h>

/*
 * This is a ptr to the current error handling function. This function is
 * responsible for clearing the "error_value" stack after printing the
 * error / warning.
 */
void (*error_handler)(void) = NULLPTR;

/*
 * The place where the last error occurred. This might be pointing to an
 * intermediate token or other types of values which has to be determined based
 * on the current phase of compilation and the error type.
 */
stack error_value;

const char* error_message[] = { "Failed to allocate memory",\
"Failed to reallocate memory", "Failed to open file" };

/* This clears the "error_value" stack. */
void clear_error_value_stack()
{
    while (!STACK_IS_EMPTY(error_value))
        stack_pop(&error_value);
}

#if CHECK_MALLOC_RETURNS
/*
 * This handles a lot of common errors that don't need any extra processing
 * like failing to allocate memory.
 */
void handle_common_error(u32 error_code)
{
    printf("%s.\n", error_message[error_code]);
    #if DEBUG
    abort();
    #else
    exit(-1);
    #endif
}
#endif

/* This handles sending custom error messages. */
void send_error(char* error_message)
{
    printf("%s %s\n", ERROR_STRING, error_message);
    fflush(stdout);
    #if DEBUG
    abort();
    #else
    exit(-1);
    #endif
}