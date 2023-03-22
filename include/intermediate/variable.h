/* This file handles intermediate variables. */

#ifndef INTERMEDIATE_VARIABLE_H
#define INTERMEDIATE_VARIABLE_H

#include<intermediate/pass.h>

/* These are the flags given to variable symbols. */
typedef enum variable_symbol_flag {
    VARIABLE_SYMBOL_REGISTER = 1,
    VARIABLE_SYMBOL_GLOBAL = 2,
    VARIABLE_SYMBOL_ON_STACK = 4,
    VARIABLE_SYMBOL_CONST = 8,
    VARIABLE_SYMBOL_EXTERNAL = 16,
    // TODO: Restrict
} variable_symbol_flag;

/* struct variable_symbol - This is the symbol of a single variable
 * @extra_data: This is extra data defined by the front end
 * @type: The type of this variable
 * @hash: The hash of this variable's name
 * @array_size: If runtime array checks are enabled this is the length of the
 * array otherwise this is not present
 * @flags: The flags this variable has
 * @uses: The number of times this variable has been used
 */
typedef struct variable_symbol {
    void* extra_data;
    type type;
    u32 hash;
    variable_symbol_flag flags;
    #if RUNTIME_ARRAY_BOUND_CHECKS
    u32 array_size;
    #endif
    u8 uses;
} variable_symbol;

/*
 * This returns a ptr to the variable symbol from the inputted variable name's
 * hash or a NULLPTR if a variable with the inputted hash wasn't found.
 */
variable_symbol* get_variable_symbol(intermediate_pass* _pass, u32 hash);

/*
 * This returns a ptr to the variable symbol from the inputted variable name or
 * a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol_from_name(intermediate_pass* _pass, \
char* name);

/*
 * This adds a variable symbol to the inputted intermediate pass' variables from
 * the inputted data. Returns a ptr to the variable symbol and in cases of
 * errors returns a NULLPTR.
 */
variable_symbol* add_variable_symbol(intermediate_pass* _pass, char* name, \
type type, u8 flags);

/*
 * This adds the inputted variable symbol to the inputted intermediate pass'
 * variables this doesn't copy the variable symbol from the ptr it just inserts
 * the ptr into the variable hash table.
 */
void add_variable_symbol_ptr(intermediate_pass* _pass, \
variable_symbol* _variable);

/*
 * This removing all of the variable symbols from the inputted intermediate
 * pass' variable hash table. This doesn't free the variable symbols as variable
 * intermediates have ownership over them.
 */
void clear_variables_in_scope(intermediate_pass* _pass);

/*
 * This frees the heap memory used by the inputted intermediate pass' variable
 * symbol table.
 */
static inline void free_variables(intermediate_pass* _pass)
{
    clear_variables_in_scope(_pass);
    free(_pass->variables.contents);
}

#endif