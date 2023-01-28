/*
 * This does the first pass of intermediate optimization which invloves the
 * analysis of basic blocks.
 */

#ifndef BACKEND_INTERMEDIATE_OPTIMIZATION_REGISTERPASS_H
#define BACKEND_INTERMEDIATE_OPTIMIZATION_REGISTERPASS_H

#include<common.h>

/*
 * This function performs the first intermediate optimization pass. This returns
 * a pointer to the optimized intermediates.
 */
void optimization_do_register_pass();

#endif
