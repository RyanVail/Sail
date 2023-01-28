/*
 * This optimization pass goes through all of the intermediates and evalutes
 * arithments invloving constants.
 */

#ifndef BACKEND_INTERMEDIATE_OPTIMIZATION_CONSTANTPASS_H
#define BACKEND_INTERMEDIATE_OPTIMIZATION_CONSTANTPASS_H

#include<common.h>
#include<main.h>
#include<intermediate/intermediate.h>

/*
 * This goes through all of the the constants in the intermediates and evalutes
 * them. Constants as in values inside of "CONST" / "CONSTPTR" intermediates.
 */
void optimization_do_constant_pass();

#endif