/*
 * This handles everything to do with the symbol table.
 */

#ifndef BACKEND_INTERMEDIATE_SYMBOLTABLE
#define BACKEND_INTERMEDIATE_SYMBOLTABLE

#include<common.h>
#include<types.h>

/* struct function_symbol - This is the symbol of a single function
 * @inputs: A vector of the variables that are an input this this function
 * @return_type: This is the type this function returns
 * @calls: This is the number of times this function has been called
 * @defintion: 0 -> Defined, 1 -> To-Be Defined, 2 -> External
 * @hash: The hash of this function's name
 */
typedef struct function_symbol {
    vector inputs;
    type return_type;
    u8 calls;
    u8 defintion;
    u32 hash;
} function_symbol;

/* struct variable_symbol - This is the symbol of a single variable
 * @type: The type of this variable
 * @hash: The hash of this variable's name
 * @flags: 1 -> register, 2 -> global, 4 -> on stack
 * @uese: The number of times this variable has been used
 */
typedef struct variable_symbol {
    type type;
    u32 hash;
    // TODO: Flags should be an enum same defintion in "function_symbol"
    u8 flags;
        // bool register;
        // bool global;
        // bool need_stack;
    u8 uses;
} variable_symbol;

/*
 * This initializes symbol tables vectors.
 */
void init_symbol_table(u8 function_init_size, u8 variable_init_size);

/*
 * This adds a function symbol to the symbol table. Returns true if adding the
 * function symbol was a success.
 */
bool add_function_symbol(char* name, vector inputs, type _return, u8 defintion);

/*
 * This returns a pointer to the function symbol if found or a null pointer
 */
function_symbol* get_function_symbol(char* name, u32 hash);

/*
 * This returns a pointer to the variable symbol if found or a null pointer
 */
variable_symbol* get_variable_symbol(char* name, u32 hash);

/*
 * This adds a variable symbol to the symbol tabel. Returns true if adding the
 * variable symbol was a success.
 */
bool add_variable_symbol(char* name, type type, u8 flags);

/*
 * This adds a variable symbol pointer to the symbol table.
 */
void add_variable_symbol_ptr(variable_symbol* _variable_symbol);

/*
 * This clears all variables from the symbol table that are in scope without
 * freeing their variable symbol structs since they should be pointed to by the
 * "VAR_DECLERATION" intermediates.
 */
void clear_variables_in_scope();

/*
 * This inits the symbol table.
 */
void init_symbol_table(u8 function_init_size, u8 variable_init_size);

/*
 * This frees all the memory that is token up by the symbol table
 */
void free_symbol_table();

#endif