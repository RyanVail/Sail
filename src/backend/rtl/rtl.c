/*
 * This is the level between the intermediates and the machine code / assembly
 * generation level. rtl instructions are general instructions that are then
 * translated into machine dependent instructions based on the target.
 */

#define USE_RTL_OPERAND

#include<backend/rtl/rtl.h>

// TODO: All of these functions should be in the rtl header file.

/*
 * This returns the index of a free register. If no free register is found this
 * will create a new register in the register vector.
 */
u32 rtl_get_free_reg(intermediate_pass* _pass)
{
    /* Trying to find a non used register*/
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = &VECTOR_AT(&_pass->data.rtl->regs, i, reg);
        if (_reg->content.type == NIL)
            return i;
    }

    /* Adding a new register. */
    reg new_reg = {
        .type = RTL_NIL,
    };

    vector_append(&_pass->data.rtl->regs, &new_reg);
    return VECTOR_SIZE(_pass->data.rtl->regs) - 1;
}

// TODO: These O(n^2) things like this should use a small hash table.
/*
 * This returns the register index of the register that contains the actual copy
 * of the inputted variable. In the case that no register is found this will
 * return UINT32_MAX.
 */
u32 rtl_get_reg_with_variable(intermediate_pass* _pass, variable_symbol* _var)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = &VECTOR_AT(&_pass->data.rtl->regs, i, reg);
        if (_reg->content.type == VAR_DECLARATION && _reg->content.ptr == _var)
            return i;
    }
}

// TODO: This should also remove the variable symbol.
/*
 * This ignores the inputted variable by removing the register that it is
 * defined in.
 */
void rtl_ignore_variable(intermediate_pass* _pass, intermediate _intermediate)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = &VECTOR_AT(&_pass->data.rtl->regs, i, reg);
        if (_reg->content.type == VAR_DECLARATION && _reg->content.ptr \
        == _intermediate.ptr)
            _reg->content.type = RTL_NIL;
    }
}

/*
 * This copies the inputted struct into a register in rtl instruction and
 * return the index of the register.
 */
u32 rtl_copy_struct(intermediate_pass* _pass)
{
    // TODO:
}

/*
 * This puts a copy of the inputted variable into a register in rtl instructions
 * and returns the index of the register it's put into. This doesn't copy
 * structs use "rtl_copy_struct" for that. This will not add an the variable
 * operand onto the operand stack.
 */
u32 rtl_copy_variable(intermediate_pass* _pass, variable_symbol* _var)
{
    #if DEBUG
    if (_var->type.kind == STRUCT_TYPE)
        send_error("rtl_copy_variable cannot be used to copy structs");
    if (rtl_get_reg_with_variable(_pass, _var) == UINT32_MAX)
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
            .content.reg = UINT32_MAX,
        },
        .destination = rtl_get_free_reg(_pass),
        .type = RTL_MOV,
    };

    /* Adding the instruction. */
    vector_append(&_pass->data.rtl->rtls, &instruction);
    return VECTOR_SIZE(_pass->data.rtl->rtls);
}

/* This clears all of the defined rtl labels and regs. */
void clear_rtl(intermediate_pass* _pass)
{
    return; // TODO:
}

/*
 * This loads the inputted variable access intermediate into an intermediate
 * register and returns the index of the register. This will also create the rtl
 * instruction that moves the variable into the register. This doesn't work for
 * structs use "rtl_load_struct" for that.
 */
u32 rtl_load_variable(intermediate_pass* _pass, intermediate _intermediate)
{
    /* Adding the rtl instruction and register content. */
    u32 reg = rtl_copy_variable(_pass, _intermediate.ptr);

    /* Creating the operand. */
    operand _operand = {
        .immediate_or_reg = RTL_OPERAND_REG,
        .type = ((variable_symbol*)_intermediate.ptr)->type,
        .content.reg = reg,
    };

    /* Adding the operand to onto the operand stack. */
    stack_push(&_pass->operand_stack, &_operand);

    return reg;
}

// TODO: This should add the variable to the intermediate pass' variable symbols
// and the symbol should contain the variable register index.
/*
 * This gives the variable inside of the inputted intermediate decleration an
 * inital rtl register. This will not create any rtl instructions. This doesn't
 * work for structs use "rtl_declarwe_struct" for that.
 */
u32 rtl_declare_variable(intermediate_pass* _pass, intermediate _intermediate)
{
    /* Creating the operand. */
    operand _operand = {
        .content.immediate = _intermediate,
        .type = ((variable_symbol*)_intermediate.ptr)->type,
    };

    /* Placing the operand into the register. */
    u32 reg = rtl_get_free_reg(_pass);
    VECTOR_AT(&_pass->data.rtl->regs, reg, operand) = _operand;

    return reg;
}

/*
 * This loads the inputted constant intermediate into an intermediate register
 * and returns the index of the register. This will also create the rtl
 * instruction that moves the constant into the register.
 */
u32 rtl_load_const(intermediate_pass* _pass, intermediate _intermediate)
{
    i64 value;
    #if !PTRS_ARE_64BIT
    value = (_intermediate.type == CONST) ? (i64)(size_t)_intermediate.ptr
    : *(i64*)_intermediate.ptr;
    #else
    value = (i64)(size_t)_intermediate.ptr;
    #endif

    /* Creating the operand. */
    operand _operand = {
        .content.immediate = _intermediate,
        .type = {
            .extra_data = 0,
            .kind = get_lowest_type(value),
        },
    };

    /* Placing the operand into the register. */
    u32 reg = rtl_get_free_reg(_pass);
    VECTOR_AT(&_pass->data.rtl->regs, reg, operand) = _operand;

    /* Making the rtl instrcution. */
    rtl_instruction instruction = {
        .destination = reg,
        .operand0 = {
            .immediate_or_reg = RTL_OPERAND_IMMEDIATE,
            .content = _intermediate,
        },
        .type = RTL_MOV,
    };

    /* Adding the rtl instruction and returning the rgister index. */
    vector_append(&_pass->data.rtl->rtls, &instruction);
    return reg;
}

/*
 * This creates and adds an rtl instruction based on the inputted operand and
 * the inputted const, with the constant being operand1.
 */
void rtl_operate_on_reg_and_const(intermediate_pass* _pass, u32 reg,
num constant)
{
    /* Creating the inital instruction. */
    rtl_instruction instruction = {
        .destination = rtl_get_free_reg(_pass),
        .operand0 = {
            .immediate_or_reg = RTL_OPERAND_REG,
            .content.reg = reg,
        },
        .operand1 = {
            .immediate_or_reg = RTL_OPERAND_IMMEDIATE,
        },
        .type = RTL_ADD,
    };

    /* Placing the constant into the operand0's intermediate. */
    set_intermediate_to_const(&instruction.operand0.content.immediate,constant);

    vector_append(&_pass->data.rtl->rtls, &instruction);
}

/*
 * This creates and adds an rtl instruction based on the inputted operand and
 * the inputted const, with the constant being operand0.
 */
void rtl_operate_on_const_and_reg(intermediate_pass* _pass, u32 reg,
num constant)
{
    /* Creating the inital instruction. */
    rtl_instruction instruction = {
        .destination = rtl_get_free_reg(_pass),
        .operand0 = {
            .immediate_or_reg = RTL_OPERAND_IMMEDIATE,
        },
        .operand1 = {
            .immediate_or_reg = RTL_OPERAND_REG,
            .content.reg = reg,
        },
        .type = RTL_ADD,
    };

    /* Placing the constant into the operand0's intermediate. */
    set_intermediate_to_const(&instruction.operand0.content.immediate,constant);

    vector_append(&_pass->data.rtl->rtls, &instruction);
}

/* This translates and returns the inputted intermediates in rtl form. */
intermediate_pass intermediates_into_rtl(vector* intermediates)
{
    /*
     * Any intermediate values that are operands as mapped into rtl operand
     * types and placed into the pass' operand stack which can then be placed
     * into "rtl_instruction"s directly without any extra processing.
     */

    // TODO: This has to init the values in the pass.
    // TODO: This intermediate pass needs to take the function symbols from the
    // past intermediate passes.
    intermediate_pass pass;
    intermediate _intermediate;

    rtl_instruction instruction;
    operand* operand0;
    operand* operand1;
    for (u32 i=0; i < VECTOR_SIZE(*intermediates); i++) {
        _intermediate = *(intermediate*)vector_at(intermediates, i, false);
        switch (_intermediate.type)
        {
        case INC:
        case DEC:
            rtl_operate_on_const_and_reg (
                &pass,
                ((operand*)stack_top(&pass.operand_stack))->content.reg,
                (num) {
                    .magnitude = 1,
                    .negative = (_intermediate.type),
                }
            );
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case AND:
        case XOR:
        case OR:
        case LSL:
        case LSR:
        case MOD:
        case NOT_EQUAL:
        case IS_EQUAL:
        case GREATER_THAN:
        case GREATER_THAN_EQUAL:
        case LESS_THAN:
        case LESS_THAN_EQUAL:
            instruction.operand0 = *(operand*)stack_pop(&pass.operand_stack);
            instruction.operand1 = *(operand*)stack_top(&pass.operand_stack);
            break;
        #if !PTRS_ARE_64BIT
        case CONST_PTR:
        #endif
        case CONST:
            rtl_load_const(&pass, _intermediate);
            break;
        case VAR_ACCESS:
            rtl_load_variable(&pass, _intermediate);
            break;
        case VAR_DECLARATION:
            rtl_declare_variable(&pass, _intermediate);
            break;
        // TODO: Here ignore is only effecting one intermediate.
        case IGNORE:
            rtl_ignore_variable(&pass, _intermediate);
            break;
        }
        instruction.destination = rtl_get_free_reg(&pass);
    }

    return pass;
}

/* This will clear all temp rtl values in registers. */
void rtl_clear_tmp_values_in_scope(intermediate_pass* _pass)
{
    reg* _reg;
    for (u32 i=0; i < VECTOR_SIZE(_pass->data.rtl->regs); i++) {
        _reg = &VECTOR_AT(&_pass->data.rtl->regs, i, reg);
        switch (_reg->content.type)
        {
        #if PTRS_ARE_64BIT
        case CONST_PTR:
        #endif
        case CONST:
        case FLOAT:
        case DOUBLE:
        case VAR_RETURN:
        case FUNC_RETURN:
        case VAR_ACCESS:
        case VAR_DECLARATION:
            _reg->content.type = NIL;
        }
    }
}

// TODO:
/*
 * This is an array of the default rtl instruction handler functions. There are
 * the functions that get run when the rtl running function is a NULLPTR.
 */
static const rtl_instruction_func default_rtl_instruction_funcs[RTL_NIL] = {
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
    NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR, NULLPTR,
};