/*
 * This optimization pass goes through all of the intermediates and evaluates
 * arithmetics involving constants.
 */

#include<intermediate/optimization/constantpass.h>
#include<evaluate.h>
#include<types.h>

/* This returns the constant pass data of the inputted intermediate pass ptr. */
#define PASS_DATA(pass_ptr) (((constant_pass_data*)pass_ptr->data.extra))

/* This is the communativity of an operation. */
typedef enum commutativity {
    NONE,
    NEGATION,
    ADD_SUB,
    MUL_DIV,
} commutativity;

/*
 * struct constant_pass_data - This is the extra data attached to an
 * intermediate pass for the constant intermediate pass
 * @last_communtativity: The commutativity of the last operation
 * @result_type: This is the resulting type of the operation
 */
typedef struct constant_pass_data {
    commutativity last_communtativity;
    type result_type;
} constant_pass_data;

/* This process an int operation for the constant optimization pass. */
void constant_optimization_process_operation(intermediate* operand0, \
intermediate* operand1, intermediate* _current, type result_type)
{
    i64 result;

    #if PTRS_ARE_64BIT
    if (!(operand0->type == CONST && operand1->type == CONST))
        return;
    #else
    if (!(((operand0->type == CONST || operand0->type == CONST_PTR)
    && (operand1->type == CONST || operand1->type == CONST_PTR))))
        return;
    #endif
    /* Evaluating the constant expression. */
    #if PTRS_ARE_64BIT
    result = evaluate_expression((i64)operand1->ptr, (i64)operand0->ptr, \
        _current->type);
    #else
    i64* operand0_value;
    i64* operand1_value;
    /* Freeing the operands after getting them. */
    if (operand0->type == CONST_PTR) {
        operand0_value = *((i64*)operand0->ptr);
        free(operand0->ptr);
    } else if (operand0->type == CONST) {
        operand0_value = &operand0->ptr;
    }

    if (operand1->type == CONST_PTR) {
        operand1_value = (i64)operand1->ptr;
        operand1->ptr = NULLPTR;
    } else {
        operand1_value = &operand1->ptr;
    }

    result = evaluate_expression(*operand1_value, *operand0_value,\
        _current->type);
    #endif

    operand0->type = NIL;
    _current->type = NIL;

    // TODO: For ptrs this needs to do something different.
    /* Scaling the result. */
    result = scale_value_to_type(result, result_type);

    /* Adding the result back to the intermediates. */
    #if !PTRS_ARE_64BIT
    set_intermediate_to_const(operand1, result);
    #else
    operand1->ptr = (void*)result;
    #endif
}

/*
 * This is an intermediate handler function that adds the inputted intermediate
 * to the operand stack.
 */
void _add_operand_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    stack_push(&_pass->operand_stack, _intermediate);
}

/*
 * This is an intermediate handler function that removes an operand from the
 * operand stack.
 */
void _pop_operand_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    stack_pop(&_pass->operand_stack);
}

/*
 * This is an intermediate handler function that handles multiplying and
 * dividing operations.
 */
void _mul_div_operand_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    stack_pop(&_pass->operand_stack);
    PASS_DATA(_pass)->last_communtativity = MUL_DIV;
}

/*
 * This is an intermediate handler function that handles adding and subtracting
 * operations.
 */
void _add_sub_operand_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    intermediate* operand0 = stack_pop(&_pass->operand_stack);
    if (PASS_DATA(_pass)->last_communtativity == ADD_SUB \
    || PASS_DATA(_pass)->last_communtativity == NONE)
        constant_optimization_process_operation(operand0, \
        stack_top(&_pass->operand_stack), _intermediate, \
        PASS_DATA(_pass)->result_type);
    PASS_DATA(_pass)->last_communtativity = ADD_SUB;
}

/* This is an intermediate handler function that handles casting. */
void _cast_operand_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    #if PTRS_ARE_64BIT
    PASS_DATA(_pass)->result_type = *(type*)(&_intermediate->ptr);
    #else
    PASS_DATA(_pass)->result_type = *((type*)_intermediate->ptr);
    #endif
}

/*
 * This is an intermediate handler function that handles poping all operands for
 * clear stack intermediates.
 */
void _clear_stack_handler_func(intermediate_pass* _pass, intermediate* \
_intermediate)
{
    while (!STACK_IS_EMPTY(_pass->operand_stack))
        stack_pop(&_pass->operand_stack);
    PASS_DATA(_pass)->result_type.kind = U32_TYPE;
    PASS_DATA(_pass)->result_type.ptr_count = 0;
    PASS_DATA(_pass)->last_communtativity = NONE;
}

// TODO: Sometimes these small comments start with //, sometimes they have
// periods, and other times they don't make it consistent.

// TODO: This should work with floats, doubles, and other operations not just
// adding and subtracting.

/* This is an array of the constant pass intermediate handler functions. */
static const ptr_handler_func \
const_handler_funcs[INTERMEDIATE_TYPE_NORMAL_END+1] = {
    /* INC through NEG */
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,

    /* ADD and SUB */
    &_add_sub_operand_handler_func, &_add_sub_operand_handler_func,

    /* MUL and DIV */
    &_mul_div_operand_handler_func, &_mul_div_operand_handler_func,

    /* AND through MOD */
    &_pop_operand_handler_func, &_pop_operand_handler_func,
    &_pop_operand_handler_func, &_pop_operand_handler_func,
    &_pop_operand_handler_func, &_pop_operand_handler_func,

    /* NOT_EQUAL through LESS_THAN_EQUAL. */
    &_pop_operand_handler_func, &_pop_operand_handler_func,
    &_pop_operand_handler_func, &_pop_operand_handler_func,
    &_pop_operand_handler_func, &_pop_operand_handler_func,

    /* EQUAL intermediates */
    &_clear_stack_handler_func,

    /* VAR intermediates */
    &_add_operand_handler_func, &_add_operand_handler_func,
    &_add_operand_handler_func, &_clear_stack_handler_func,

    /* MEM intermediates */
    &_clear_stack_handler_func, &_clear_stack_handler_func,

    /* Program flow intermediates */
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func, &_clear_stack_handler_func,

    /* CONST and CONST_PTR intermediates */
    &_add_operand_handler_func, &_add_operand_handler_func,

    /* FLOAT and DOUBLE intermediates */
    &_clear_stack_handler_func, &_clear_stack_handler_func,

    /* Struct variables and RETURN temp intermediates */
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func,

    /* CAST */
    &_cast_operand_handler_func,

    /* Variable optimization intermediates */
    &_clear_stack_handler_func, &_clear_stack_handler_func,
    &_clear_stack_handler_func,

    /* CLEAR_STACK */
    &_clear_stack_handler_func,
};

/*
 * This goes through all of the the constants in the intermediates and evaluates
 * them. Constants as in values inside of "CONST" / "CONSTPTR" intermediates.
 * This also removes / simplifies things like double negatives. This pass will
 * leave NILs in the intermediates.
 */
void optimization_do_constant_pass(intermediate_pass* _pass)
{
    /* Initing the constant pass' values of the intermediate pass. */
    _pass->data.extra = malloc(sizeof(constant_pass_data));
    CHECK_MALLOC(PASS_DATA(_pass));
    PASS_DATA(_pass)->last_communtativity = NONE;
    _pass->operand_stack.top = NULLPTR;
    _pass->recreate = false;

    /* Settings the handler functions. */
    _pass->handler_funcs = (handler_func*)const_handler_funcs;

    START_PROFILING("do the constant optimization pass", \
    "do all optimization passes");

    do_intermediate_pass(_pass);
    free(PASS_DATA(_pass));

    END_PROFILING("do the constant optimization pass", true);
}
