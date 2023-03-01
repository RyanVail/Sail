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

#if DEBUG
#define CHECK_HASH_TABLES_HAVE_BEEN_INITTED() \
    if (function_symbols.contents == NULLPTR) \
        send_error("Function's symbol hash table hasn't been initted"); \
    if (variable_symbols.contents == NULLPTR) \
        send_error("Variable's symbol hash table hasn't been initted");
#else
#define CHECK_HASH_TABLES_HAVE_BEEN_INITTED()
#endif

hash_table function_symbols = { 0, NULLPTR };
hash_table variable_symbols = { 0, NULLPTR };

/*
 * This returns a pointer to the function symbol if found or a null pointer
 */
function_symbol* get_function_symbol(char* name, u32 hash)
{
    CHECK_HASH_TABLES_HAVE_BEEN_INITTED();

    /* Does one get fired for writing code like this? */
    hash_table_bucket* function_bucket = name[0] == '\0' ? \
    hash_table_at_hash(&function_symbols, hash) : \
    hash_table_at_string(&function_symbols, name);

    return function_bucket == NULLPTR ? NULL : function_bucket->value;
}

/*
 * This returns a pointer to the variable symbol if found or a null pointer
 */
variable_symbol* get_variable_symbol(char* name, u32 hash)
{
    CHECK_HASH_TABLES_HAVE_BEEN_INITTED();

    /* Does one get fired for writing code like this? */
    hash_table_bucket* variable_bucket = name[0] == '\0' ? \
    hash_table_at_hash(&variable_symbols, hash) : \
    hash_table_at_string(&variable_symbols, name);

    return variable_bucket == NULLPTR ? NULL : variable_bucket->value;
}

/*
 * This adds a variable symbol to the symbol table. Returns true if adding the
 * variable symbol was a success.
 */
bool add_variable_symbol(char* name, type type, u8 flags)
{
    CHECK_HASH_TABLES_HAVE_BEEN_INITTED();

    if (get_variable_symbol(name, 0) != NULLPTR)
        return false;

    variable_symbol* _variable = malloc(sizeof(variable_symbol));
    CHECK_MALLOC(_variable);

    _variable->flags = flags;
    _variable->uses = 0;
    _variable->type = type;

    hash_table_bucket* variable_bucket = \
        hash_table_insert_string(&variable_symbols, name);

    _variable->hash = variable_bucket->hash;
    variable_bucket->value = _variable;

    return true;
}

/*
 * This adds a variable symbol pointer to the symbol table.
 */
void add_variable_symbol_ptr(variable_symbol* _variable_symbol)
{
    hash_table_bucket* variable_bucket = \
        hash_table_insert_hash(&variable_symbols, _variable_symbol->hash);

    variable_bucket->value = _variable_symbol;
}

/*
 * This adds a function symbol to the symbol table. Inputs has to be on the
 * heap. Returns true if adding the function symbol was a success.
 */
bool add_function_symbol(char* name, vector inputs, type _return, u8 defintion)
{
    CHECK_HASH_TABLES_HAVE_BEEN_INITTED();

    if (get_function_symbol(name, 0))
        return false;

    function_symbol* _function = malloc(sizeof(function_symbol));
    CHECK_MALLOC(_function);

    _function->inputs = inputs;
    _function->return_type = _return;
    _function->calls = 0;
    _function->defintion = defintion;

    hash_table_bucket* function_bucket = \
        hash_table_insert_string(&function_symbols, name);

    _function->hash = function_bucket->hash;
    function_bucket->value = _function;

    return true;
}

/*
 * This clears all variables from the symbol table that are in scope without
 * freeing their variable symbol structs since they should be pointed to by the
 * "VAR_DECLARATION" intermediates.
 */
void clear_variables_in_scope()
{
    hash_table_bucket* current_bucket = variable_symbols.contents;
    hash_table_bucket* linked_bucket = NULLPTR;
    for (u32 i=0; i < (1 << variable_symbols.size); i++) {
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

/*
 * This inits the symbol table.
 */
void init_symbol_table(u8 function_init_size, u8 variable_init_size)
{
    function_symbols = hash_table_init(function_init_size);
    variable_symbols = hash_table_init(variable_init_size);
}

/* This clears all entries in the symbol table. */
void clear_symbol_table()
{
    hash_table_bucket* current_bucket = variable_symbols.contents;
    hash_table_bucket* linked_bucket = NULLPTR;

    current_bucket = function_symbols.contents;

    // TODO: All of these dumb * sizeof(hash_table_bucket) should not exist
    // anymore but a lot of code relieses on it.
    for (; (u8*)current_bucket < (u8*)function_symbols.contents + \
    sizeof(hash_table_bucket) * (1 << function_symbols.size); current_bucket++)
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

/* This frees all the memory that is token up by the symbol table. */
void free_symbol_table()
{
    clear_symbol_table();

    free(function_symbols.contents);
    free(variable_symbols.contents);
}