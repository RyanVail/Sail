/*
 * This file contains untilites to help make passes through the intermediates
 * easier and remove a lot of copy-pasted code. Intermediate passes in general
 * is documentated in "documentation/passes/pass.md".
 */

#include<intermediate/pass.h>
#include<intermediate/intermediate.h>

/*
 * These functions are wrappers over normal intermediate functions that need
 * special arguments so they can be transformed into function ptrs.
 */
// TODO: There should just be a single function per intermediate type that
// actually accepts a stack and intermediate.
void _process_operation(intermediate_pass* _pass, intermediate _intermediate)
{
    process_operation(_pass, _intermediate.type);
}
/*
 * This is a wrapper over "add_cast_intermediate" which is just used as a
 * function ptr becuase "add_cast_intermediate" doesn't accept an intermediate.
 */
void _add_cast_intermediate(intermediate_pass* _pass, intermediate \
_intermediate)
{
    #if PTRS_ARE_64BIT
    add_cast_intermediate(_pass, *(type*)&_intermediate.ptr);
    #else
    add_cast_intermediate(_pass, _intermediate.ptr);
    #endif
}

// TODO: There should be a function to handle the end of an intermediate pass
// ie. freeing the symbol tables and things.
// TODO: These default ones aren't right.
/*
 * This is an array of the default intermediate handler functions. These are the
 * functions that get run when the function ptr in the handler function is a
 * NULLPTR.
 */
static const handler_func \
default_handler_functions[INTERMEDIATE_TYPE_NORMAL_END+1] = {
    /* Operations */
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,
    &_process_operation, &_process_operation, &_process_operation,

    // TODO: The intermediate logic of defining a variable should be in the
    // intermediate file rather than the front ends.
    /* Var */
    &_add_back_intermediate, &_add_back_intermediate, &_add_back_intermediate,
    &_add_back_intermediate,

    /* Memory */
    &_process_operation, &_process_operation,

    // TODO: There should be logic for statments in the intermediate file
    /* Statments */
    &_add_back_intermediate, &_add_back_intermediate, &_add_back_intermediate,
    &_add_back_intermediate, &_add_back_intermediate, &_add_back_intermediate,

    // TODO: There should be logic for functions in the intermediate file
    /* Functions */
    &_add_back_intermediate, &_add_back_intermediate,

    /* Gotos */
    &_add_back_intermediate,

    // TODO: There isn't a shared function to add a constant and
    // "set_intermediate_to_const" alone isn't enough
    /* Constant values */
    &_add_back_intermediate, &_add_back_intermediate, &_add_back_intermediate,
    &_add_back_intermediate,

    /* Struct variable */
    &_add_back_intermediate,

    /* Temp return intermediates */
    &_skip_intermediate, &_skip_intermediate, &_skip_intermediate,
    &_skip_intermediate,

    /* Casts */
    &_add_cast_intermediate,

    /* Register */
    &_add_back_intermediate, &_add_back_intermediate, &_add_back_intermediate,
    &_add_back_intermediate,

    /* Clear stack */
    &_skip_intermediate,
};

/*
 * This adds the inputted intermediate to the inputted intermediate pass and
 * preforms the normal intermediate pass logic based on the inputted
 * intermediate type. This should be used during front end parsing.
 */
void add_intermediate_to_pass(intermediate_pass* _pass, \
intermediate _intermediate)
{
    vector_append(&_pass->intermediates, &_intermediate);

    /* Getting the intermediate handler function. */
    handler_func _func = _pass->handler_funcs[_intermediate.type];
    if (_func == NULLPTR)
        _func = default_handler_functions[_intermediate.type];

    /* Calling the intermediate handler function. */
    (*_func)(_pass, _intermediate);
}

/* This preforms the inputted intermediate pass. */
void do_intermediate_pass(intermediate_pass* _pass)
{
    handler_func _func;
    intermediate* _intermediate;
    vector* intermediates = &_pass->intermediates;

    // TODO: This "recreate" pass flag needs to be fully implemented with
    // different default handler functions.

    /* Reseting the pass' intermedaites if needed. */
    if (_pass->recreate)
        _pass->intermediates = vector_init(sizeof(intermediate), \
        intermediates->size);

    for (u32 i=0; i < VECTOR_SIZE(*intermediates); i++) {
        /* Getting the current intermediate. */
        _intermediate = vector_at(intermediates, i, 0);

        /* NILs are skipped. */
        if (_intermediate->type == NIL)
            continue;

        /* Getting the intermediate handler function. */
        _func = _pass->handler_funcs[_intermediate->type];
        if (_func == NULLPTR) {
            if (!_pass->recreate)
                continue;
            _func = default_handler_functions[_intermediate->type];
        }

        // TODO: This should try and call the special intermediate handler
        // functions.

        // printf("calling: %p, for: %u\n", _func, _intermediate->type);
        // fflush(stdout);

        /* Calling the intermediate handler function. */
        if (_pass->recreate)
            (*_func)(_pass, *_intermediate);
        else
            (*(ptr_handler_func)_func)(_pass, _intermediate);
    }

    // TODO: Free the old "intermediates" if "recreate".
}

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function adds the inputted intermediate back into the intermediate
 * vector preforming no logic at all.
 */
void _add_back_intermediate(intermediate_pass* _pass, intermediate \
_intermediate)
{
    add_back_intermediate(_pass, _intermediate);
}

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function just returns doing no logic. This should be used when the
 * default intermediate function handler's logic should be skipped. This will
 * not add the intermediate back use the func "_add_back_intermediate" for that.
 */
void _skip_intermediate(intermediate_pass* _pass, intermediate _intermediate)
{
    return;
}