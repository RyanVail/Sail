/*
 * This handles everything to do with the symbol table.
 */

#ifndef INTERMEDIATE_SYMBOLTABLE
#define INTERMEDIATE_SYMBOLTABLE

#include<common.h>
#include<types.h>
#include<datastructures/hash_table.h>

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
 * This adds a function symbol with the inputted values to the inputted function
 * symbol table and returns a ptr to. In case of an problem this will return a
 * NULLPTR.
 */
function_symbol* add_function_symbol(hash_table* _functions, char* name, \
vector inputs, type _return, u8 definition);

/*
 * This returns a pointer to the function symbol from the inputted function hash
 * or a NULLPTR if the function wasn't found.
 */
function_symbol* get_function_symbol(hash_table* _functions, u32 hash);

/*
 * This returns a pointer to the function symbol from the inputted function name
 * or a NULLPTR if the function wasn't found.
 */
function_symbol* get_function_symbol_from_name(hash_table* _functions, \
char* name);

/*
 * This returns a pointer to the variable symbol from the inputted variable hash
 * or a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol(hash_table* _variables, u32 hash);

/*
 * This returns a pointer to the variable symbol from the inputted variable name
 * or a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol_from_name(hash_table* _variables, \
char* name);

/*
 * This adds a variable symbol to the inputted symbol tabel. Returns a ptr to
 * the variable symbol and in cases of errors returns a NULLPTR.
 */
variable_symbol* add_variable_symbol(hash_table* _variables, char* name, \
type type, u8 flags);

/*
 * This adds the inputted variable symbol to the inputted symbol table this
 * doesn't copy the variable symbol from the ptr it just inserts the ptr into
 * the hash table.
 */
void add_variable_symbol_ptr(hash_table* _variables, variable_symbol* \
_variable);

/*
 * This clears all variables from the inputted symbol table that are in scope
 * without freeing their variable symbol structs since they should be pointed
 * to by the "VAR_DECLARATION" intermediates.
 */
void clear_variables_in_scope(hash_table* _variables);

/* This clears all entries from the inputted symbol tables. */
void clear_symbol_tables(hash_table* _variables, hash_table* _functions);

/* This frees all the memory that is token up by the inputted symbol tables. */
static inline void free_symbol_table(hash_table* _variables, \
hash_table* _functions)
{
    clear_symbol_tables(_variables, _functions);

    free(_functions->contents);
    free(_variables->contents);
}

#endif