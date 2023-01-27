/* This file does all of the evaluating of equations pre computation. */

#include<frontend/common/evaluate.h>

// TODO: This only supports a left to right operands.
/* This evaluates the inputed expression and returns the result. */
i64 evaluate_expression(i64 operand0, i64 operand1, operator _operator)
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
    #endif
    }
}