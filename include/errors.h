#ifndef ERRORS_H
#define ERRORS_H

#include<common.h>

/*
 * This handles a lot of common errors that don't need any extra processing
 * like failing to allocate memory.
 */
void handle_error(u32 error_code);

/*
 * This handles sending custom error messages.
 */
void send_error(char* error_message);

#endif
