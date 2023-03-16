/*
 * This file contains untilites to help make passes through the intermediates
 * easier and remove a lot of copy-pasted code. Intermediate passes and how
 * front ends should interact with them is documented in
 * "documentation/intermediate/passes.md"
 */
// TODO: There should be some way to add more types to this that works in the
// same function ptr way intermediate handlers work

#ifndef INTERMEDIATE_PASS_H
#define INTERMEDIATE_PASS_H

#include<common.h>
#include<intermediate/types.h>
#include<datastructures/stack.h>
#include<datastructures/hash_table.h>

typedef struct intermediate_pass intermediate_pass;

/* This is the types of intermediate handler functions. */
typedef void (*handler_func)(intermediate_pass*, intermediate);

/*
 * This maps the inputted intermediate type to the inputted function pointer
 * through either the "intermediate_handler_functions" array or if it is not
 * normal the "special_intermediate_handler_funcions". When the intermediate
 * type is reached in a pass the function will be called.
 */
#define INTERMEDIATE_PASS_SET_FUNC(_pass, _type, _func_ptr) \
    if (_type > INTERMEDIATE_TYPE_NORMAL_END) { \
        intermediate_pass_set_special_handler_func(_pass, _type, _func_ptr); \
    } else { \
        _pass->handler_funcs[_type] = _func_ptr; \
    }

/*
 * This sets the index of the inputted type into the special function handler
 * vector to the inputted function ptr. This doesn't do any checks to make sure
 * the inputted index of the vector is present.
 */
#define INTERMEDIATE_PASS_SET_SET_SPECIAL_FUNC_AT(_pass, _type, _func_ptr) \
    *(handler_func*)vector_at(&_pass->special_handler_funcs, _type, false) \
    = _func_ptr

// TODO: Maybe these should have periods so long blocks of text like this one
// can use something other than commas.

/* struct intermediate_pass - This struct represents all of the specifications
 * of an intermediate pass and values needed during an intermediate pass
 * @intermediates: This is the vector of intermediates
 * @operand_stack: This is the operand stack
 * @handler_funcs: This is an array of the intermediate handler functions, these
 * functions when an intermediate is read the "intermediate_type" is indexed
 * into this array and the returing function ptr is called, in the case the
 * function ptr is a NULLPTR the default handler function will be called.
 * @special_handler_funcs: This is a vector of intermediate handler functions,
 * these work in the same way "handler_funcs" work but with a dynamic array so
 * the indexes between the desired index and the last index might need to be
 * filled with NULLPTR. This will do no logic when reaching a NULLPTR func ptr.
 * @function_symbols: This is a hash table of "function_symbol" structs as
 * defined in "symboltable.h".
 * @variable_symbols: This is a hash table of "variable_symbol" structs as
 * defined in "symboltable.h".
 * @typedefs: This is the typedef hash table see "intermediate/typedef.c".
 * @structs: This is the struct hash table see "intermediate/struct.c".
 * @enums: This is the enums hash table see "intermediate/enum.c".
 */
typedef struct intermediate_pass {
    /* Intermediates */
    vector intermediates;
    stack operand_stack;
    /* Handler functions */
    handler_func handler_funcs[INTERMEDIATE_TYPE_NORMAL_END];
    vector special_handler_funcs;
    /* Symbol table data */
    hash_table function_symbols;
    hash_table variable_symbols;
    /* Typedefs, structs, and enums */
    hash_table typedefs;
    hash_table structs;
    hash_table enums;
} intermediate_pass;

/* This creates and returns a new intermediate pass. */
static inline intermediate_pass init_intermediate_pass()
{
    intermediate_pass _pass;
    memset(&_pass, 0, sizeof(intermediate_pass));
    _pass.intermediates.type_size = sizeof(intermediate);

    return _pass;
}

/*
 * This sets the inputted intermediate type index in the special intermediate
 * type handler function to the inputted function ptr. This shouldn't be called
 * directly but through the "INTERMEDIATE_PASS_SET_FUNC" macro.
 */
static inline void intermediate_pass_set_special_handler_func( \
intermediate_pass* _pass, intermediate_type _type, handler_func _func)
{
    /* The size of the special vector. */
    u32 special_size = VECTOR_SIZE(_pass->special_handler_funcs);

    /* If the vector already has enough space for this function. */
    if (special_size < _type) {
        INTERMEDIATE_PASS_SET_SET_SPECIAL_FUNC_AT(_pass, _type, _func);
        return;
    }

    /* If this vector doesn't have enough space append the needed NULLPTRs. */
    void* _nullptr = NULLPTR;
    for (u32 i=0; i < _type - INTERMEDIATE_TYPE_NORMAL_END - special_size; i++)
        vector_append(&_pass->special_handler_funcs, &_nullptr);

    vector_append(&_pass->special_handler_funcs, &_func);
}

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function adds the inputted intermediate back into the intermediate
 * vector preforming no logic at all.
 */
void _add_back_intermediate(intermediate_pass* _pass, intermediate \
_intermediate);

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function just returns doing no logic. This should be used when the
 * default intermediate function handler's logic should be skipped. This will
 * not add the intermediate back use the func "_add_back_intermediate" for that.
 */
void _skip_intermediate(intermediate_pass* _pass, intermediate _intermediate);

#endif