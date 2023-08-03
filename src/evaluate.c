/* This file does all of the evaluating of equations pre computation. */

#include<evaluate.h>
#include<types.h>
#include<intermediate/intermediate.h>

/* This evaluates the inputted u64 expression and returns the result. */
u64 evaluate_u64_expression(u64 operand0, u64 operand1, operator _operator)
{
    switch(_operator)
    {
    case ADD:
        
        return operand0 + operand1;
    case SUB:
        return operand0 - operand1;
    case MUL:
        return operand0 * operand1;
    case DIV:
        return operand0 / operand1;
    case AND:
        return operand0 & operand1;
    case XOR:
        return operand0 ^ operand1;
    case OR:
        return operand0 | operand1;
    case LSL:
        return operand0 << operand1;
    case LSR:
        return operand0 >> operand1;
    case MOD:
        return operand0 % operand1;
    case IS_EQUAL:
        return operand0 == operand1;
    case NOT_EQUAL:
        return operand0 != operand1;
    case GREATER_THAN:
        return operand0 > operand1;
    case GREATER_THAN_EQUAL:
        return operand0 >= operand1;
    case LESS_THAN:
        return operand0 < operand1;
    case LESS_THAN_EQUAL:
        return operand0 <= operand1;
    #if DEBUG
    default:
        send_error("Unknown operation");
    #else
    default:
        __builtin_unreachable();
    #endif
    }
}

/* This evaluates the inputted i64 expression and returns the result. */
i64 evaluate_i64_expression(i64 operand0, i64 operand1, operator _operator)
{
    switch(_operator)
    {
    case ADD:
        return operand0 + operand1;
    case SUB:
        return operand0 - operand1;
    case MUL:
        return operand0 * operand1;
    case DIV:
        return operand0 / operand1;
    case AND:
        return operand0 & operand1;
    case XOR:
        return operand0 ^ operand1;
    case OR:
        return operand0 | operand1;
    case LSL:
        return operand0 << operand1;
    case LSR:
        return operand0 >> operand1;
    case MOD:
        return operand0 % operand1;
    case IS_EQUAL:
        return operand0 == operand1;
    case NOT_EQUAL:
        return operand0 != operand1;
    case GREATER_THAN:
        return operand0 > operand1;
    case GREATER_THAN_EQUAL:
        return operand0 >= operand1;
    case LESS_THAN:
        return operand0 < operand1;
    case LESS_THAN_EQUAL:
        return operand0 <= operand1;
    #if DEBUG
    default:
        send_error("Unknown operation");
    #else
    default:
        __builtin_unreachable();
    #endif
    }
}

// TODO: Replace the
/*
 * This evaluates the inputted operation between the two inputs with the
 * inputted integer type.
 */
num evaluate_expression(num input0, num input1, type_kind _type_kind,
operator _operator)
{
    #if DEBUG
    // TODO: Make sure type kind is numerical int type.
    #endif

    num result = {
        .negative = false,
    };

    switch (_type_kind)
    {
    case U8_TYPE:
        result.magnitude = evaluate_u64_expression (
            input0.magnitude,
            input1.magnitude,
            _operator
        ) % UINT8_MAX;
        break;
    case I8_TYPE:
        result.magnitude = evaluate_i64_expression (
            input0.negative ? -input0.magnitude : input0.magnitude,
            input1.negative ? -input1.magnitude : input1.magnitude,
            _operator
        );

        result.negative = (result.magnitude & (1 << 7)) >> 7;
        result.magnitude %= ~INT8_MAX;
        break;
    case U16_TYPE:
        result.magnitude = evaluate_u64_expression (
            input0.magnitude,
            input1.magnitude,
            _operator
        ) % UINT16_MAX;
        break;
    case I16_TYPE:
        result.magnitude = evaluate_i64_expression (
            input0.negative ? -input0.magnitude : input0.magnitude,
            input1.negative ? -input1.magnitude : input1.magnitude,
            _operator
        );

        result.negative = (result.magnitude & (1 << 15)) >> 15;
        result.magnitude %= ~INT16_MAX;
        break;
    case U32_TYPE:
        result.magnitude = evaluate_u64_expression (
            input0.magnitude,
            input1.magnitude,
            _operator
        ) % UINT32_MAX;
        break;
    case I32_TYPE:
        result.magnitude = evaluate_i64_expression (
            input0.negative ? -input0.magnitude : input0.magnitude,
            input1.negative ? -input1.magnitude : input1.magnitude,
            _operator
        );

        result.negative = (result.magnitude & (1 << 31)) >> 31;
        result.magnitude %= ~INT32_MAX;
        break;
    case U64_TYPE:
        result.magnitude = evaluate_u64_expression (
            input0.magnitude,
            input1.magnitude,
            _operator
        );
        break;
    case I64_TYPE:
        *(i64*)&result.magnitude = evaluate_i64_expression (
            input0.negative ? -input0.magnitude : input0.magnitude,
            input1.negative ? -input1.magnitude : input1.magnitude,
            _operator
        );

        result.negative = (*(u64*)&result.magnitude & ((u64)1 << 63)) >> 63;
        break;
    default:
        __builtin_unreachable();
    }

    return result;
}