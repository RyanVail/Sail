/*
 * This does a pass after the basic block pass and adds ignore intermediates to
 * "REGISTER" intermediates when a variable is not used within a scope anymore.
 */

#ifndef INTERMEDIATE_OPTIMIZATION_IGNORE_PASS_H
#define INTERMEDIATE_OPTIMIZATION_IGNORE_PASS_H

#include<common.h>
#include<intermediate/intermediate.h>

/*
 * This passes through the intermediates adding "IGNORE" intermediates when
 * variables are no longer used.
 */
void optimization_do_ignore_pass();

#endif