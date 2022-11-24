/*
 * This handles everything to do with the symbol table.
 */

#ifndef BACKEND_INTERMEDIATE_SYMBOLTABLE
#define BACKEND_INTERMEDIATE_SYMBOLTABLE

#include<common.h>
#include<types.h>

/* struct function_symbol - This is the symbol of a single function
 * @name: The name of this function
 * @inputs: A vector of the variables that are an input this this function
 * @return_type: This is the type this function returns
 * @calls: This is the number of times this function has been called
 * @defintion: 0 -> Defined, 1 -> To-Be Defined, 2 -> External
 * @id: The id of the function
 */
typedef struct function_symbol {
    char* name;
        // TODO: Figure out if this needs to be a pointer or not.
    vector inputs;
    type return_type;
    u8 calls;
    u8 defintion;
    u32 id;
} function_symbol;

/* struct variable_symbol - This is the symbol of a single variable
 * @name: The name of this variable
 * @type: The type of this variable
 * @id: The id of the variable
 * @flags: 1 -> in scope, 2 -> used, 4 -> register, 8 -> global
 * @uese: The number of times this variable has been used
 */
typedef struct variable_symbol {
    char* name;
    type type;
    u32 id;
    // TODO: Flags should be an enum same defintion in "function_symbol"
    u8 flags;
        // bool used;
        // bool register;
        // bool global;
    u8 uses;
} variable_symbol;

/*
 * This initializes symbol tables vectors.
 */
void init_symbol_table();

/*
 * This adds a function symbol to the symbol table. Returns true if adding the
 * function symbol was a success.
 */
bool add_function_symbol(char* name, vector inputs, type _return, u8 defintion);

/*
 * This returns a pointer to the function symbol if found or a null pointer
 */
function_symbol* get_function_symbol(char* name, u32 id);

/*
 * This returns a pointer to the variable symbol if found or a null pointer
 */
variable_symbol* get_variable_symbol(char* name, u32 id);

/*
 * This adds a variable symbol to the symbol tabel. Returns true if adding the
 * variable symbol was a success.
 */
bool add_variable_symbol(char* name, type type, u8 flags);

/*
 * This frees all the memory that is token up by the symbol table
 */
void free_symbol_table();

#endif