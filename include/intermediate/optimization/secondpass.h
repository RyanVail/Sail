/*
 * This does the second pass of intermediate optimization which invloves the
 * replacment of "VAR_ACCESS" with "VAR_ASSIGNMENT" in needed places. aswell as
 * evaluation of constant expressions.
 */

#include<common.h>
#include<intermediate/intermediate.h>
#include<datastructures/stack.h>

/*
 * Pointer width can be set by external code using the following line of code:
 * extern u8 target_pointer_width = x;
 */

inline static i64 second_pass_shrink_const(i64 result, type _type);

inline static i64 second_pass_evaluate_const(intermediate_type operation, \
i64 first, i64 second);

/*
 * This function performs the second intermediate optimization pass. This
 * returns a pointer to the optimized intermediates.
 */
void optimization_do_second_pass();