/*
 * This handles everything to do with the symbol table.
 */

#include<backend/intermediate/symboltable.h>

vector function_symbols = { NULL, 0, 0, sizeof(function_symbol) };
vector variable_symbols = { NULL, 0, 0, sizeof(variable_symbol) };

/* 
 * This is a vector of all variables names in order of ids. This is used to
 * print the name of a variable if it was used outside of its scope.
 */
vector variable_names = { NULL, 0, 0, sizeof(char*) };

/*
 * This returns a pointer to the function symbol if found or a null pointer
 */
function_symbol* get_function_symbol(char* name, u32 id)
{
    if (name != NULL) {
        for (u32 i=0; i < VECTOR_SIZE(function_symbols); i++) {
            function_symbol* _func = \
                (function_symbol*)vector_at(&function_symbols, i, false);

            if (!strcmp(_func->name, name))
                return _func;
        }
    } else {
        for (u32 i=0; i < VECTOR_SIZE(function_symbols); i++) {
            function_symbol* _func = \
                (function_symbol*)vector_at(&function_symbols, i, false);

            if (_func->id == id)
                return _func;
        }
    }
}

/*
 * This returns a pointer to the variable symbol if found or a null pointer
 */
variable_symbol* get_variable_symbol(char* name, u32 id)
{
    if (name != NULL) {
        for (u32 i=0; i < VECTOR_SIZE(variable_symbols); i++) {
            variable_symbol* _func = \
                (variable_symbol*)vector_at(&variable_symbols, i, false);

            if (!strcmp(_func->name, name))
                return _func;
        }
    } else {
        for (u32 i=0; i < VECTOR_SIZE(variable_symbols); i++) {
            variable_symbol* _func = \
                (variable_symbol*)vector_at(&variable_symbols, i, false);

            if (_func->id == id)
                return _func;
        }
    } 
    return NULL;
}

/*
 * This adds a variable symbol to the symbol table. Returns true if adding the
 * variable symbol was a sucess.
 */
bool add_variable_symbol(char* name, type type, u8 flags)
{
    if (VECTOR_SIZE(variable_symbols) && get_variable_symbol(name, 0))
        return false;

    u32 id = VECTOR_SIZE(variable_names);

    char* _name = malloc(sizeof(char) * strlen(name));
    strcpy(_name, name);
    if (_name == NULL)
        handle_error(0);

    variable_symbol _variable = { _name, type, id, flags, 0 };
    
    vector_append(&variable_symbols, &_variable);
    vector_append(&variable_names, &_name);

    return true;
}

/*
 * This adds a function symbol to the symbol table. Returns true if adding the
 * function symbol was a success.
 */
bool add_function_symbol(char* name, vector inputs, type _return, u8 defintion)
{
    if (get_function_symbol(name, NULL))
        return false;

    u32 id = ((function_symbol*)vector_at( \
        &function_symbols, VECTOR_SIZE(function_symbols)-1, false))->id+1;

    function_symbol _function = { name, inputs, _return, 0, defintion, id };
    vector_append(&function_symbols, &_function);
    return true;
}

/*
 * This frees all the memory that is token up by the symbol table
 */
void free_symbol_table()
{
    if (variable_names.contents != NULL) {
        // TODO: Benchmark this against a for loop through every item
        while (VECTOR_SIZE(variable_names)) {
            char** _t = vector_pop(&variable_names);
            free(*_t);
            free(_t);
        }
        free(variable_names.contents);
    }

    /* 
     * The variable names are the same as in "variable_names" so we don't need
     * to free them twice. 
     */
    if (variable_symbols.contents != NULL)
        free(variable_symbols.contents);

    if (function_symbols.contents != NULL)
        free(function_symbols.contents);
}