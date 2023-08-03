/*
 * This is an intermediate pass that ensures a variable has been inited before
 * being used. This tracks the mask through bit wise operations but other types
 * of arithmetics make the bit mask invalid along with pointers.
 */

#define INTERMEDIATE_WARNING_ASSIGNMENT_C

#include<intermediate/warning/assignment.h>

/* This returns a ptr to the init mask of the inputted variable. */
#define VAR_DATA(var_ptr) ((init_mask*)var_ptr->extra_data)

/* This will give the top operand a fuzzy init mask.*/
void _fuzzy_top_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    ((operand*)stack_top(&_pass->operand_stack))->fuzzy = true;
}

/* This will pop the top operand and give the next top a fuzzy init mask. */
void _pop_and_fuzzy_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    free(stack_pop(&_pass->operand_stack));
    ((operand*)stack_top(&_pass->operand_stack))->fuzzy = true;
}

// TODO: This should be implemented in "pass.c" and be called
// "_clear_operand_handler_func".
/* This pops all operands from the inputted intermediate pass' operand stack. */
void _clear_operands_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    while (!STACK_IS_EMPTY(_pass->operand_stack))
        free(stack_pop(&_pass->operand_stack));
}

/* This is an array of the assignemnt warning pass handler functions. */
static const ptr_handler_func \
assignemnt_handler_funcs[INTERMEDIATE_TYPE_NORMAL_END+1] = {
    /* INC through NEG */
    &_fuzzy_top_handler_func, &_fuzzy_top_handler_func,
    &_fuzzy_top_handler_func, &_fuzzy_top_handler_func,
    &_fuzzy_top_handler_func,

    /* ADD through LESS_THAN_EQUAL */
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,
    &_pop_and_fuzzy_handler_func, &_pop_and_fuzzy_handler_func,

    /* EQUAL */
    NULLPTR, // TODO:

    /* VAR */
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, // TODO:

    /* MEM */
    NULLPTR, NULLPTR, // TODO: Location = fuzzy and Deref = Check last bit mask

    /* PROGRAM FLOW */
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,

    /* CONST and CONST_PTR */
    NULLPTR, NULLPTR, // TODO:

    /* FLOAT and DOUBLE */
    NULLPTR, NULLPTR, // TODO:

    /* Struct variables and RETURN temps */
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, // TODO: This should be able to
                                                 // track structs.

    /* CAST */
    NULLPTR, // TODO:

    /* Variable optimization */
    NULLPTR, NULLPTR, NULLPTR,

    /* CLEAR_STACK */
    &_clear_operands_handler_func,
};

/*
 * This will preform a pass through the intermediate in the inputted
 * intermediate pass and return a vector of "var_non_assignment"s which can be
 * used by front ends to display warnings.
 */
vector warning_do_assignment_pass(intermediate_pass* _pass)
{
    //
}