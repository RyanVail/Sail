#include<errors.h>

const char* error_message[] = { "Failed to allocate memory",\
"Failed to reallocate memory", "Failed to open file" };

/*
 * This handles a lot of common errors that don't need any extra processing
 * like failing to allocate memory.
 */
void handle_error(u32 error_code)
{
    printf("%s.\n", error_message[error_code]);
    #if DEBUG
    abort();
    #else
    exit(-1);
    #endif
}

/*
 * This handles sending custom error messages.
 */
void send_error(char* error_message)
{
    printf("%s.\n", error_message);
    #if DEBUG
    abort();
    #else
    exit(-1);
    #endif
}