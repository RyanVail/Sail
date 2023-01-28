/* This file does all of the evaluating of equations pre computation. */

#ifndef FRONTEND_COMMON_EVALUATE_H
#define FRONTEND_COMMON_EVALUATE_H

#include<intermediate/intermediate.h>
#include<common.h>

/*
 * Operators ids are the same as those of "intermediate_type" expect for
 * "OPENING_PAR" and "CLOSING_PAR" which are just temp anyway.
 */
typedef u8 operator;

#define OPENING_PAR 22
#define CLOSING_PAR 23

/* The logically highest value of the "operator" type. */
#define __OPERATOR_MAX__ ((operator)-1)

/* This evaluates the inputed expression and returns the result. */
i64 evaluate_expression(i64 input0, i64 input1, operator _operator);

#endif