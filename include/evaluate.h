/* This file does all of the evaluating of equations pre computation. */

#ifndef EVALUATE_H
#define EVALUATE_H

#include<intermediate/intermediate.h>
#include<common.h>

/*
 * Operators ids are the same as those of "intermediate_type" expect for
 * "OPENING_PAR" and "CLOSING_PAR" which are just temp anyway.
 */
typedef u8 operator;

#define OPENING_PAR ((operator)22)
#define CLOSING_PAR ((operator)23)

/* The logically highest value of the "operator" type. */
#define __OPERATOR_MAX__ ((operator)-1)

/* This evaluates the inputted expression and returns the result. */
i64 evaluate_expression(i64 input0, i64 input1, operator _operator);

#endif