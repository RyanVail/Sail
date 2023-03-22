/* This file handles intermediate variables. */

#ifndef INTERMEDIATE_FUNCTION_H
#define INTERMEDIATE_FUNCTION_H

#include<intermediate/pass.h>

/* These are flags given to function symbols. */
typedef enum function_symbol_flag {
    FUNCTION_SYMBOL_FLAG_FORWARD_DEF = 1,
    FUNCTION_SYMBOL_EXTERNAL = 2,
} function_symbol_flag;

/* struct function_symbol - This is the symbol of a single function
 * @extra_data: This is extra data defined by the front end
 * @inputs: A vector of the variables that are an input this this function
 * @extra_data: This is extra data attached to this function attached to it by
 * frontends, and used by front ends
 * @hash: The hash of this function's name
 * @return_type: This is the type this function returns
 * @calls: This is the number of times this function has been called
 * @defintion: 0 -> Defined, 1 -> To-Be Defined, 2 -> External
 */
typedef struct function_symbol {
    void* extra_data;
    vector inputs;
    u32 hash;
    type return_type;
    u8 calls;
    u8 defintion;
} function_symbol;

/*
 * This adds a function symbol with the inputted values to the inputted
 * intermediate pass' functions using the inputted data. Returns a ptr to the
 * created function symbol and a NULLPTR if there was an error.
 */
function_symbol* add_function_symbol(intermediate_pass* _pass, char* name, \
vector inputs, type _return, u8 definition);

/*
 * This returns a pointer to the function symbol from the inputted intermediate
 * pass' functions or a NULLPTR if it wasn't found.
 */
function_symbol* get_function_symbol(intermediate_pass* _pass, u32 hash);

/*
 * This returns a pointer to the function symbol from the inputted function name
 * and intermediate pass or a NULLPTR if it wasn't found.
 */
function_symbol* get_function_symbol_from_name(intermediate_pass* _pass, \
char* name);

/* This clears the function symbol table from the inputted intermediate pass. */
void clear_function_symbol_table(intermediate_pass* _pass);

/*
 * This frees the heap memory used by the inputted intermediate pass' function
 * symbol table.
 */
static inline void free_functions(intermediate_pass* _pass)
{
    clear_function_symbol_table(_pass);
    free(_pass->functions.contents);
}

#endif