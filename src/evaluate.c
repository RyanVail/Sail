/* This file does all of the evaluating of equations pre computation. */

#include<evaluate.h>
#include<types.h>

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

/* This simplifies the inequality at the inputed intermediate vector. */
void simplify_inequalitiy(intermediate* starting_interemdiate)
{
    /*
    (3x) > (x + 1)
    (3 x *) (x 1 +) >               RPN
    ( (3 x *) ) ( (x) 1 + ) >       SEPERATE TERMS
    ( (2 x *) ) ( 0 1 + ) >         SUBTRACT SMALLER COMMON TERM
    ( (2 x *) 0 1 + - ) (0) >       SUBTRACT OPERAND ZERO BY OPERAND ONE
    ( 2 x * 1 - ) (0) >             CONSTANT PASS


    (2x + 3 - 4) <= (2x - 4)
    (2 x * 3 + 4 -) (2 x * 4 -) <=          RPN
    ( (2 x *) 3 + 4 -) ( (2 x *) 4 -) <=    SEPERATE COMMON TERMS
    (0 3 + 4 -) (0 4 -) <=                  REPLACE COMMON TERMS WITH ZERO
    (0)                                     CONSTANT PASS


    ( 3 x * 2 +) ( 2 y * 3 -) >         RPN
    ( (3 x *) 2 +) ( (2 y *) 3 -) >     SEPERATE TERMS
    ( (3 x *) (2 y *) - 2 + 3 -) (0) >  MOVING EVERYTHING TO OPERAND ZERO
    ( 3 x * 2 y * - 1 -) (0) >          CONSTANT PASS
    */
}