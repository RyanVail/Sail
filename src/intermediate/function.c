/* This file handles intermediate variables. */

/* This file handles intermediate variables. */

#include<types.h>
#include<datastructures/hash_table.h>
#include<intermediate/function.h>

/*
 * This adds a function symbol with the inputted values to the inputted
 * intermediate pass' functions using the inputted data. Returns a ptr to the
 * created function symbol and a NULLPTR if there was an error.
 */
function_symbol* add_function_symbol(intermediate_pass* _pass, char* name, \
vector inputs, type _return, u8 definition)
{
    /* Making sure the function isn't already in the hash table. */
    if (get_function_symbol_from_name(_pass, name))
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
    hash_table_bucket* function_bucket = \
        hash_table_insert_string(&_pass->functions, name);
    _function->hash = function_bucket->hash;
    function_bucket->value = _function;

    return function_bucket->value;
}


/*
 * This returns a pointer to the function symbol from the inputted intermediate
 * pass' functions or a NULLPTR if it wasn't found.
 */
function_symbol* get_function_symbol(intermediate_pass* _pass, u32 hash)
{
    hash_table_bucket* function_bucket = hash_table_at_hash(&_pass->functions, \
        hash);
    return function_bucket == NULLPTR ? NULLPTR : function_bucket->value;
}

/*
 * This returns a pointer to the function symbol from the inputted function name
 * and intermediate pass or a NULLPTR if it wasn't found.
 */
function_symbol* get_function_symbol_from_name(intermediate_pass* _pass, \
char* name)
{
    hash_table_bucket* function_bucket = \
        hash_table_at_string(&_pass->functions, name);
    return function_bucket == NULLPTR ? NULLPTR : function_bucket->value;
}

/* This clears the function symbol table from the inputted intermediate pass. */
void clear_function_symbol_table(intermediate_pass* _pass)
{
    hash_table_bucket* current_bucket = _pass->functions.contents;
    hash_table_bucket* linked_bucket = NULLPTR;

    // TODO: All of these dumb * sizeof(hash_table_bucket) should not exist
    // anymore but a lot of code relieses on it.
    for (; (u8*)current_bucket < (u8*)_pass->functions.contents + \
    sizeof(hash_table_bucket) * (1 << _pass->functions.size); current_bucket++)
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