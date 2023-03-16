/*
 * This handles everything to do with the symbol table. These functions should
 * be used during the tokens to intermediate stage.
 */
// TODO: The "add" functions should return the hash of the variable or function
// symbol instead of a bool so a "get" function shouldn't have to be called
// after creating the function or variable.

#include<datastructures/hash_table.h>
#include<intermediate/symboltable.h>
#include<types.h>

/*
 * This adds a function symbol with the inputted values to the inputted function
 * symbol table and returns a ptr to. In case of an problem this will return a
 * NULLPTR.
 */
function_symbol* add_function_symbol(hash_table* _functions, char* name, \
vector inputs, type _return, u8 definition)
{
    /* Making sure the function isn't already in the hash table. */
    if (get_function_symbol_from_name(_functions, name))
        return NULLPTR;

    /* Allocating the function symbol. */
    function_symbol* _function = malloc(sizeof(function_symbol));
    CHECK_MALLOC(_function);

    /* Initing the function symbol. */
    _function->inputs = inputs;
    _function->return_type = _return;
    _function->calls = 0;
    _function->defintion = definition;

    /* Adding the hash. */
    hash_table_bucket* function_bucket = hash_table_insert_string(_functions, \
        name);
    _function->hash = function_bucket->hash;
    function_bucket->value = _function;

    return function_bucket->value;
}

/*
 * This returns a pointer to the function symbol from the inputted function hash
 * or a NULLPTR if the function wasn't found.
 */
function_symbol* get_function_symbol(hash_table* _functions, u32 hash)
{
    hash_table_bucket* function_bucket = hash_table_at_hash(_functions, hash);
    return function_bucket == NULLPTR ? NULLPTR : function_bucket->value;
}

/*
 * This returns a pointer to the function symbol from the inputted function name
 * or a NULLPTR if the function wasn't found.
 */
function_symbol* get_function_symbol_from_name(hash_table* _functions, \
char* name)
{
    hash_table_bucket* function_bucket = hash_table_at_string(_functions, name);
    return function_bucket == NULLPTR ? NULLPTR : function_bucket->value;
}

/*
 * This returns a pointer to the variable symbol from the inputted variable hash
 * or a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol(hash_table* _variables, u32 hash)
{
    hash_table_bucket* variable_bucket = hash_table_at_hash(_variables, hash);
    return variable_bucket == NULLPTR ? NULLPTR : variable_bucket->value;
}

/*
 * This returns a pointer to the variable symbol from the inputted variable name
 * or a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol_from_name(hash_table* _variables, \
char* name)
{
    hash_table_bucket* variable_bucket = hash_table_at_string(_variables, name);
    return variable_bucket == NULLPTR ? NULLPTR : variable_bucket->value;
}

/*
 * This adds a variable symbol to the inputted symbol tabel. Returns a ptr to
 * the variable symbol and in cases of errors returns a NULLPTR.
 */
variable_symbol* add_variable_symbol(hash_table* _variables, char* name, \
type type, u8 flags)
{
    /* Making sure the variable symbol isn't already present. */
    if (get_variable_symbol_from_name(_variables, name) != NULLPTR)
        return NULLPTR;

    /* Allocating the variable symbol. */
    variable_symbol* _variable = malloc(sizeof(variable_symbol));
    CHECK_MALLOC(_variable);

    /* Initing the variable symbol. */
    _variable->flags = flags;
    _variable->uses = 0;
    _variable->type = type;

    /* Adding the hash. */
    hash_table_bucket* variable_bucket = hash_table_insert_string(_variables, \
        name);
    _variable->hash = variable_bucket->hash;
    variable_bucket->value = _variable;

    return variable_bucket->value;
}

/*
 * This adds the inputted variable symbol to the inputted symbol table this
 * doesn't copy the variable symbol from the ptr it just inserts the ptr into
 * the hash table.
 */
void add_variable_symbol_ptr(hash_table* _variables, variable_symbol* _variable)
{
    hash_table_bucket* variable_bucket = hash_table_insert_hash(_variables, \
    _variable->hash);

    variable_bucket->value = _variable;
}

/*
 * This clears all variables from the inputted symbol table that are in scope
 * without freeing their variable symbol structs since they should be pointed
 * to by the "VAR_DECLARATION" intermediates.
 */
void clear_variables_in_scope(hash_table* _variables)
{
    hash_table_bucket* current_bucket = _variables->contents;
    hash_table_bucket* linked_bucket = NULLPTR;
    for (u32 i=0; i < (1 << _variables->size); i++) {
        if (current_bucket->next != NULLPTR) {
            linked_bucket = current_bucket->next;
            do {
                void* tmp = linked_bucket->next;
                free(linked_bucket);
                linked_bucket = tmp;
            } while (linked_bucket != NULLPTR);
        }
        current_bucket->value = 0;
        current_bucket->next = 0;
        current_bucket->hash = 0;
        current_bucket++;
    }
}

/* This clears all entries from the inputted symbol tables. */
void clear_symbol_tables(hash_table* _variables, hash_table* _functions)
{
    hash_table_bucket* current_bucket = _variables->contents;
    hash_table_bucket* linked_bucket = NULLPTR;

    current_bucket = _functions->contents;

    // TODO: All of these dumb * sizeof(hash_table_bucket) should not exist
    // anymore but a lot of code relieses on it.
    for (; (u8*)current_bucket < (u8*)_functions->contents + \
    sizeof(hash_table_bucket) * (1 << _functions->size); current_bucket++)
    {
        if (current_bucket->next != NULLPTR) {
            linked_bucket = current_bucket->next;
            do {
                if (current_bucket->value != NULLPTR) {
                    function_symbol* _func = current_bucket->value;
                    if (_func->inputs.contents != NULLPTR)
                        free(_func->inputs.contents);
                    free(linked_bucket->value);
                }
                linked_bucket = linked_bucket->next;
                linked_bucket->value = NULLPTR;
                linked_bucket->hash = 0;
            } while (linked_bucket->next != NULLPTR);
        }
        if (current_bucket->value != NULLPTR) {
            function_symbol* _func = current_bucket->value;
            if (_func->inputs.contents != NULLPTR)
                free(_func->inputs.contents);
            free(current_bucket->value);
            current_bucket->hash = 0;
            current_bucket->value = NULLPTR;
        }
    }
}