/*
 * This is an intermediate pass that ensures a variable has been inited before
 * being used. This tracks the mask through bit wise operations but other types
 * of arithmetics make the bit mask fuzzy along with pointers.
 */

#ifndef INTERMEDIATE_WARNING_ASSIGNMENT_H
#define INTERMEDIATE_WARNING_ASSIGNMENT_H


/*
 * If this is included from the C file of this pass the operand struct needs to
 * be redefined for this pass.
 */
#ifdef INTERMEDIATE_WARNING_ASSIGNMENT_C
#define OPERAND_DEFINED

#include<intermediate/types.h>

/* This is used to show the bits of a value that are inited. */
typedef u64 init_mask;

/* struct assignemnt_operand - This is a redefinition of an operand that
 * contains information for the assignemnt pass
 * @origin: The intermediate that this operand comes from
 * @mask: The init mask of this operand
 * @intermediate: The value of this operand
 * @fuzzy: If the init mask of this operand is fuzzy
 * @type: The type of this operand
 */
typedef struct assignemnt_operand {
    intermediate* origin;
    init_mask mask;
    intermediate intermediate;
    bool fuzzy;
    type type;
} operand;

#else
#include<intermediate/types.h>
#endif

#include<intermediate/pass.h>

// TODO: There should be a flag to make fuzzy bit masks a warning too.

/* struct invalid_assignment - This is used for displaying warnings about
 * unitialized values
 * @initalization: The intermediate where the invalid operand comes from
 * @invalid_uses; Uses of the variable while it's uninited
 */
typedef struct invalid_assignment {
    intermediate* initalization;
    vector invalid_uses;
} invalid_assignment;

/*
 * This will preform a pass through the intermediate in the inputted
 * intermediate pass and return a vector of "var_non_assignment"s which can be
 * used by front ends to display warnings.
 */
vector warning_do_assignment_pass(intermediate_pass* _pass);

#endif