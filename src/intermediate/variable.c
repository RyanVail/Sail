/* This file handles intermediate variables. */

#include<types.h>
#include<datastructures/hash_table.h>
#include<intermediate/variable.h>

/*
 * This returns a ptr to the variable symbol from the inputted variable name's
 * hash or a NULLPTR if a variable with the inputted hash wasn't found.
 */
variable_symbol* get_variable_symbol(intermediate_pass* _pass, u32 hash)
{
    hash_table_bucket* variable_bucket = hash_table_at_hash(&_pass->variables, \
        hash);
    return variable_bucket == NULLPTR ? NULLPTR : variable_bucket->value;
}

/*
 * This returns a ptr to the variable symbol from the inputted variable name or
 * a NULLPTR if the variable wasn't found.
 */
variable_symbol* get_variable_symbol_from_name(intermediate_pass* _pass, \
char* name)
{
    hash_table_bucket* variable_bucket = \
        hash_table_at_string(&_pass->variables, name);
    return variable_bucket == NULLPTR ? NULLPTR : variable_bucket->value;
}

/*
 * This adds a variable symbol to the inputted intermediate pass' variables from
 * the inputted data. Returns a ptr to the variable symbol and in cases of
 * errors returns a NULLPTR.
 */
variable_symbol* add_variable_symbol(intermediate_pass* _pass, char* name, \
type type, u8 flags)
{
    /* Making sure the variable symbol isn't already present. */
    if (get_variable_symbol_from_name(_pass, name) != NULLPTR)
        return NULLPTR;

    /* Allocating the variable symbol. */
    variable_symbol* _variable = malloc(sizeof(variable_symbol));
    CHECK_MALLOC(_variable);

    /* Initing the variable symbol. */
    _variable->flags = flags;
    _variable->uses = 0;
    _variable->type = type;

    /* Adding the hash. */
    hash_table_bucket* variable_bucket = \
        hash_table_insert_string(&_pass->variables, name);
    _variable->hash = variable_bucket->hash;
    variable_bucket->value = _variable;

    return variable_bucket->value;
}

/*
 * This adds the inputted variable symbol to the inputted intermediate pass'
 * variables this doesn't copy the variable symbol from the ptr it just inserts
 * the ptr into the variable hash table.
 */
void add_variable_symbol_ptr(intermediate_pass* _pass, \
variable_symbol* _variable)
{
    hash_table_bucket* variable_bucket = \
        hash_table_insert_hash(&_pass->variables, _variable->hash);

    variable_bucket->value = _variable;
}

/*
 * This removing all of the variable symbols from the inputted intermediate
 * pass' variable hash table. This doesn't free the variable symbols as variable
 * intermediates have ownership over them.
 */
void clear_variables_in_scope(intermediate_pass* _pass)
{
    hash_table_bucket* current_bucket = _pass->variables.contents;
    hash_table_bucket* linked_bucket = NULLPTR;
    for (u32 i=0; i < (1 << _pass->variables.size); i++) {
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