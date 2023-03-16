/*
 * This is the level between the intermediates and the machine code / assembly
 * generation level. RTL instructions are general instructions that are then
 * translated into machine dependent instructions based on the target.
 */

#include<backend/rtl/rtl.h>

/* The register's statuses. */
static vector regs;

/*
 * This returns the register index of the register that contains the actual copy
 * of the inputted variable. In the case that no register is found this will
 * return "__UINT32_MAX__".
 */
u32 rtl_get_reg_with_variable(variable_symbol* _var)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(regs); i++) {
        _reg = vector_at(&regs, i, false);
        if (_reg->content.type == VAR_DECLARATION)
            return i;
    }
}

/*
 * This puts a copy of the inputted variable into a register in RTL instructions
 * and returns the index of the register it's put into. This will copy ptrs not
 * their data. Structs that don't need to be in the correct memory layout will
 * be put into registers.
 */
u32 rtl_copy_variable(variable_symbol* _var)
{
    u32 original_index = rtl_get_reg_with_variable(_var);
    if (IS_TYPE_INT(_var->type)) {
        //
    } else {
        //
    }
}

/* This clears all of the defined RTL labels and regs. */
void clear_rtl();