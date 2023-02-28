/*
 * This file handles the internal repersentation of inequalities and
 * operationing on those inequalites.
 */

#ifndef INTERMEDIATE_OPTIMIZATION_INEQUALITY_H
#define INTERMEDIATE_OPTIMIZATION_INEQUALITY_H

#include<common.h>
#include<intermediate/intermediate.h>
#include<evaluate.h>

/* struct inequality - This struct represents a single inequality
 * @operator: This is the operator of the inequality
 * @operand: This is the operand of the inequality
 */
typedef struct inequality {
    operator operator;
    intermediate operand;
} inequality;

/* This returns true if the operation is true if the two operands are equal. */
#define IS_EQUAL_TO_OPERATOR(x) !(x & 1)

/*
 * This returns true if the two inputted inequalities have a valid solution,
 * otherwise false.
 */
bool inequality_is_in_inequality(inequality* inequality0, \
inequality* inequality1);

/*
 * This returns true if the inputted inequality is a solution to the inputted
 * system of inequalities, otherwise false.
 */
bool inequality_is_in_system(vector* inequalities, inequality* inequality);

/*
 * This adds the inputted inequality to the inputted system of inequalities.
 * This is prefered over "vector_append" because this does additional work. The
 * returning bool will be true if there is no solution to the inequality.
 */
bool add_to_inequality(vector* inequalities, inequality* inequality);

#endif