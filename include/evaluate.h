/* This file does all of the evaluating of equations during compile time. */

#ifndef EVALUATE_H
#define EVALUATE_H

#include<intermediate/intermediate.h>
#include<common.h>

/*
 * Operators ids are the same as those of "intermediate_type" expect for the
 * operators defined in the next anonymous union.
 */
typedef u8 operator;

enum {
    OPENING_PAR = 22,
    CLOSING_PAR = 23,

    /* The logically highest value of the "operator" type. */
    OPERATOR_MAX = (operator)-1,
};

/*
 * This evaluates the inputted operation between the two inputs with the
 * inputted integer type.
 */
num evaluate_expression(num input0, num input1, type_kind _type_kind,
operator _operator);

#endif