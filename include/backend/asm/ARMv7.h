/*
 * This is the ARMv7 assembly / machine code backend.
 */

#ifndef BACKEND_ASM_ARMv7_H
#define BACKEND_ASM_ARMv7_H

#include<common.h>

#define REGISTERS_COMPLEMENT_CONST 1
#define REGISTERS_SIZE_NOT_CONST 0

#include<backend/asm/bin.h>

#undef REGISTERS_NEGATIVE_CONST
#undef REGISTERS_SIZE_NOT_CONST

#define GENERAL_REGISTER_COUNT 12
#define OPERATION_REGISTER 12
#define PROGRAM_COUNTER_REGISTER 15
#define STACK_POINTER_REGISTER 13

/*
 * This returns a "bin" which contains the outputed ARMv7 machine code from the
 * intermediates.
 */
bin ARMv7_intermediates_into_binary(vector* intermediates);

/*
 * This frees all of the dynamically allocated vectors and items in this file.
 */
void ARMv7_free_all();

#endif