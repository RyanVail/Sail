/*
 * This does the second pass of intermediate optimization which invloves the
 * replacment of "VAR_ACCESS" with "VAR_ASSIGNMENT" in needed places. aswell as
 * evaluation of constant expressions.
 */

#include<backend/intermediate/optimization/secondpass.h>

u8 target_pointer_width = __UINT8_MAX__;

static vector intermediates = { 0, 0, 0, sizeof(intermediate) };
static vector output_intermediates = { 0, 0, 0, sizeof(intermediate) };
static stack operand_stack = { 0, sizeof(operand) };

inline static i64 second_pass_shrink_const(i64 result, type _type)
{
    extern u8 target_pointer_width;
    u32* type_sizes = get_type_sizes();
    if (_type.ptr)
        return result % ((1 << (target_pointer_width << 3)) - 1);
    if (IS_TYPE_NEG(_type)) {
        // return result % ~((i64)1 << ((type_sizes[_type.kind] << 3))-1) + 1;
    } else {
        return result % (1 << (type_sizes[_type.kind] << 3))-1;
    }
}

inline static i64 second_pass_evaluate_const(intermediate_type operation, \
i64 first, i64 second)
{
    switch(operation)
    {
    case ADD:
        return second + first;
    case SUB:
        return second - first;
    case MUL:
        return second * first;
    case DIV:
        return second / first;
    case AND:
        return second & first;
    case XOR:
        return second ^ first;
    case OR:
        return second | first;
    case LSL:
        return second << first;
    case LSR:
        return second >> first;
    case MOD:
        return second % first;
    case IS_EQUAL:
        return second == first;
    case NOT_EQUAL:
        return second != first;
    case GREATER_THAN:
        return second > first;
    case GREATER_THAN_EQUAL:
        return second >= first;
    case LESS_THAN:
        return second < first;
    case LESS_THAN_EQUAL:
        return second <= first;
    }
}

/*
 * This function performs the second intermediate optimization pass. This
 * returns a pointer to the optimized intermediates.
 */
void optimization_do_second_pass()
{
    intermediates = *get_intermediate_vector();
    output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates.size);

    if (target_pointer_width == __UINT8_MAX__)
        send_error("Second pass pointer width hasn't been set");

    // TODO: This should be a pointer not an index.
    operand new_operand;
    for (u32 i=0; i < VECTOR_SIZE(intermediates); i++) {
        intermediate* _intermediate = vector_at(&intermediates, i, 0);

        if ((_intermediate->type >= ADD)
        && (_intermediate->type <= LESS_THAN_EQUAL)) {
            operand* _first = stack_top(&operand_stack);
            operand* _second = operand_stack.top->next->value;
            #if !VOID_PTR_64BIT
            if ((_first->type == CONST_PTR || _first->type == CONST)
            && (_second->type == CONST_PTR || _second->type == CONST)) {
                i64 _first_value = (i64)_first->ptr;
                i64 _second_value = (i64)_second->ptr;
                if (_first->type == CONST_PTR)
                    _first_value = *(u64*)_first_value;
                if (_second->type == CONST_PTR)
                    _second_value = *(u64*)_second_value;
            #else
            if (_first->intermediate.type == CONST
            && _second->intermediate.type == CONST) {
                i64 _first_value = _first->intermediate.ptr;
                i64 _second_value = _second->intermediate.ptr;
            #endif
                second_pass_shrink_const(second_pass_evaluate_const( \
                    _intermediate->type, _first_value, _second_value), \
                    _first->type);
                }
            stack_pop(&operand_stack);
        }

        switch (_intermediate->type)
        {
        case CLEAR_STACK:
            while (!STACK_IS_EMPTY(operand_stack))
                stack_pop(&operand_stack);
        case CONST:
            new_operand.intermediate = *_intermediate;
        }
    }
}