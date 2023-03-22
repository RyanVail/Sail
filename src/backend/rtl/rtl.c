/*
 * This is the level between the intermediates and the machine code / assembly
 * generation level. RTL instructions are general instructions that are then
 * translated into machine dependent instructions based on the target.
 */

#include<backend/rtl/rtl.h>

/* This returns the index of a free register. */
u32 rtl_get_free_reg(intermediate_pass* _pass)
{
    /* Trying to find a non used register*/
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = vector_at(&_pass->data.rtl->regs, i, false);
        if (_reg->content.type == RTL_NIL)
            return i;
    }

    /* Adding a new register. */
    reg new_reg = { .type = RTL_NIL };
    vector_append(&_pass->data.rtl->regs, &new_reg);
    return VECTOR_SIZE(_pass->data.rtl->regs) - 1;
}

/*
 * This returns the register index of the register that contains the actual copy
 * of the inputted variable. In the case that no register is found this will
 * return "__UINT32_MAX__".
 */
u32 rtl_get_reg_with_variable(intermediate_pass* _pass, variable_symbol* _var)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = vector_at(&_pass->data.rtl->regs, i, false);
        if (_reg->content.type == VAR_DECLARATION && _reg->content.ptr == _var)
            return i;
    }
}

/*
 * This ignores the inputted variable by removing the register that it is
 * defined in with the VARIABLE_DECLERATION.
 */
void rtl_ignore_variable(intermediate_pass* _pass, intermediate _intermediate)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = vector_at(&_pass->data.rtl->regs, i, false);
        if (_reg->content.type == VAR_DECLARATION && _reg->content.ptr \
        == _intermediate.ptr)
            _reg->content.type = RTL_NIL;
    }
}

/*
 * This puts a copy of the inputted variable into a register in RTL instructions
 * and returns the index of the register it's put into. This doesn't copy
 * structs use "rtl_copy_struct" for that.
 */
u32 rtl_copy_variable(intermediate_pass* _pass, variable_symbol* _var)
{
    #if DEBUG
    if (_var->type.kind == STRUCT_TYPE)
        send_error("rtl_copy_variable cannot be used to copy structs");
    if (rtl_get_reg_with_variable(_pass, _var) == __UINT32_MAX__)
        send_error("rtl_copy_variable called before variable inited");
    #endif

    /* Creating the instruction. */
    rtl_instruction instruction = {
        .operand0 = {
            .immediate_or_reg = RTL_OPERAND_REG,
            .content.reg = rtl_get_reg_with_variable(_pass, _var), 
        },
        .operand1 = {
            .immediate_or_reg = RTL_OPERAND_REG,
            .content.reg = __UINT32_MAX__,
        },
        .destination = rtl_get_free_reg(_pass),
        .type = RTL_MOV,
    };

    /* Adding the instruction. */
    vector_append(&_pass->data.rtl->rtls, &instruction);
    return VECTOR_SIZE(_pass->data.rtl->rtls);
}

/* This clears all of the defined RTL labels and regs. */
void clear_rtl()
{
    return; // TODO:
}

/* This translates and returns the inputted intermediates in RTL form. */
intermediate_pass intermediates_into_rtl(vector* intermediates)
{
    intermediate_pass pass;
    intermediate _intermediate;
    for (u32 i=0; i < VECTOR_SIZE(*intermediates); i++) {
        _intermediate = *(intermediate*)vector_at(intermediates, i, false);
        switch (_intermediate.type)
        {
        case IGNORE:
            rtl_ignore_variable(&pass, _intermediate);
            break;
        }
    }

    return pass;
}

/*
 * This is an array of the default rtl instruction handler functions. There are
 * the functions that get run when the rtl running function is a NULLPTR.
 */
static const rtl_instruction_func default_rtl_instruction_funcs[RTL_NIL] = {
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
};