/*
 * This does the first pass of intermediate optimization which invloves the
 * analysis of basic blocks.
 */

#ifndef BACKEND_INTERMEDIATE_OPTIMIZATION_FIRSTPASS_H
#define BACKEND_INTERMEDIATE_OPTIMIZATION_FIRSTPASS_H

#include<common.h>

/*
 * This function performs the first intermediate optimization pass. This returns
 * a pointer to the optimized intermediates.
 */
void optimization_do_first_pass();

#endif
