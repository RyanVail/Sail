/*
 * This is the ARMv7 assembly / machine code backend.
 *
 * When a variable is used it is put into a register and kept in that register
 * till another variable or a constant takes its place. Variables in registers
 * have the "intermediate_kind" of variable_access Registers that are being
 * operated on have the "intermediate_kind" of "VAR_RETURN".
 */

#include<backend/asm/ARMv7.h>
#include<backend/intermediate/intermediate.h>

static bin output_bin = { 0, 0 };
static reg regs[GENERAL_REGISTER_COUNT];
static vector variable_priorities = { 0, 0, 0, 0 };
static vector variable_locations = { 0, 0, 0, sizeof(u32) };
static u32 stack_pointer_location = 0;
static u8 operational_register = __UINT8_MAX__;
static u8 second_operational_register = __UINT8_MAX__;

#define ASSEMBLE_DATA_NO_SHIFT(condition, operation, flags, destination_reg, \
first_reg, second_reg) \
    condition << 28 | operation << 21 | (bool)flags << 20 | first_reg << 16 \
    | destination_reg << 12 | second_reg

/*
 * Offset is always considered negative and should be no bigger than a u12.
 * "ldr_or_str" and "byte_or_word" are in the form of "true_or_false" so true
 * in "ldr_or_str" would mean the first value or "ldr".
 */
#define ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(condition, neg, offset, ldr_or_str, \
destination_or_source_reg, byte_or_word, write_back) \
    condition << 28 | 1 << 26 | 0 << 25 | 1 << 24 | !((bool)neg) << 23 \
    | (bool)byte_or_word << 22 | (bool)write_back << 21 \
    | (bool)ldr_or_str << 20 | STACK_POINTER_REGISTER << 16 \
    | destination_or_source_reg << 12 | offset

void clear_registers();
static inline u8 get_register_with_value(intermediate _intermediate);
static inline void ARMv7_process_intermediate(intermediate _intermediate);
static inline void ARMv7_add_asm(u32 machine_code_to_add);

/*
 * This returns a "bin" which contains the outputed ARMv7 machine code from the
 * intermediates.
 */
bin intermediates_into_binary(vector* intermediates)
{
    clear_registers();
    output_bin.contents.type_size = sizeof(u32);
    for (u32 i=0; i < VECTOR_SIZE((*intermediates)); i++) {
        ARMv7_process_intermediate(*(intermediate*)vector_at(intermediates,i,0));
    }
    return output_bin;
}

/*
 * This adds the inputed machine code without a label to "output_bin".
 */
static inline void ARMv7_add_asm(u32 machine_code_to_add)
{
    vector_append(&output_bin.contents, &machine_code_to_add);
    // for (u32 i=0; i < 4; i++)
        // printf("%u\n", i);
        // printf("%u\n", &machine_code_to_add + (u32*)i);
        // printf("### %u\n", *(char*)((u32)(&machine_code_to_add) + i));
        // vector_append(&output_bin.contents, &(machine_code_to_add) + i);
}

/*
 * This saves the variable inside "reg_index" to its stack location.
 */
static inline void save_register(u8 reg_index)
{
    u32* type_sizes = get_type_sizes;

    if (regs[reg_index].content.type != VAR_ACCESS)
        return;

    variable_symbol* _var = get_variable_symbol("",regs[reg_index].content.ptr);

    if (_var->stack_location == __UINT32_MAX__)
        _var->stack_location = stack_pointer_location;
        stack_pointer_location -= type_sizes[_var->type.kind];

    ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
    _var->stack_location, true, reg_index, true, false));
}

static inline void process_register_instruction(intermediate _intermediate)
{
    variable_locations.apparent_size = 0;
    variable_priorities = *(vector*)_intermediate.ptr;
}

/*
 * This processes an intermediate token.
 */
static inline void ARMv7_process_intermediate(intermediate _intermediate)
{
    // TODO: A lot of this is repative and can be done better. An array with the
    // intermediate token types and the ARM ASM types can make for O(1).

    switch (_intermediate.type)
    {
    case INC:
    case DEC:
        /* ADD operational_register, operational_register, #1 */
        ARMv7_add_asm(14 << 28 | 1 << 25 \
        | 3 + (_intermediate.type == INC) << 21 | operational_register << 16 \
        | operational_register << 12 | 1);
        break;
    case NOT:
        /* CLZ operational_register, operational_register */
        ARMv7_add_asm(operational_register<<12|operational_register|0xe16f0f10);
        /* LSL operational_register, #5 */
        ARMv7_add_asm(14 << 28 | 13 << 21 | operational_register << 16
        | operational_register << 12 | operational_register | 5 << 7);

        regs[operational_register].content.type = BOOL_TYPE;
        break;
    case COMPLEMENT:
        break;
    case NEG:
        // TODO: Make sure "NEG" checks if the number is negative
        ARMv7_add_asm(14 << 28 | 1 << 25 | 15 << 21 |
        operational_register << 16 | operational_register << 12 | 1);
        regs[operational_register].content.type = VAR_RETURN;
        break;
    case ADD:
        printf("%u : %u\n", operational_register, second_operational_register);
        ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 4, 0, operational_register, \
        second_operational_register, operational_register));
        regs[operational_register].content.type = VAR_RETURN;
        break;
    case SUB:
        ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 2, 0, operational_register, \
        second_operational_register, operational_register));
        regs[operational_register].content.type = VAR_RETURN;
        break;
    case MUL:
        break;
    case DIV:
        break;
    case AND:
        ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 0, 0, operational_register, \
        second_operational_register, operational_register));
        regs[operational_register].content.type = VAR_RETURN;
        second_operational_register = __UINT8_MAX__;
        break;
    case XOR:
        ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 1, 0, operational_register, \
        second_operational_register, operational_register));
        regs[operational_register].content.type = VAR_RETURN;
        second_operational_register = __UINT8_MAX__;
        break;
    case OR:
        ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 12, 0, operational_register, \
        second_operational_register, operational_register));
        regs[operational_register].content.type = VAR_RETURN;
        second_operational_register = __UINT8_MAX__;
        break;
    case LSL:
        break;
    case LSR:
        break;
    case MOD:
        break;
    case IS_EQUAL:
        break;
    case NOT_EQUAL:
        break;
    case GREATER_THAN:
        break;
    case GREATER_THAN_EQUAL:
        break;
    case LESS_THAN:
        break;
    case LESS_THAN_EQUAL:
        break;
    case EQUAL:
        break;
    case VAR_ASSIGNMENT:
    case VAR_ACCESS:
        if (operational_register == __UINT8_MAX__)
            operational_register = get_register_with_value(_intermediate);
        else
            second_operational_register =get_register_with_value(_intermediate);
        break;
    case VAR_MEM:
        break;
    case MEM_LOCATION:
        break;
    case MEM_ACCESS:
        break;
    case IF:
        break;
    case ELSE:
        break;
    case LOOP:
        break;
    case END:
        break;
    case CONTINUE:
        break;
    case BREAK:
        break;
    case FUNC_CALL:
        break;
    case GOTO:
        break;
    case CONST:
    case CONST_PTR:
        if (operational_register == __UINT8_MAX__)
            operational_register = get_register_with_value(_intermediate);
        else
            second_operational_register =get_register_with_value(_intermediate);
        break;
    }
}

/*
 * This returns the priority of the inputed variable id.
 */
static inline u32 get_variable_priority(u32 variable_id)
{
    for (u32 i=0; i < VECTOR_SIZE(variable_priorities); i++)
        if (*(u32*)vector_at(&variable_priorities,i,false) == variable_id)
            return i;

    return __UINT32_MAX__;
}

/*
 * This puts the inputed constant into the desired register.
 */
static inline void put_const_into_register(i64 const_value, u8 _reg)
{
    printf("Put %u into %u\n", const_value, _reg);
}

// TODO: Variables can be swapped if they're the same size to use less memory.
// TODO: This doesn't account for half words.
/*
 * This returns the register index into "regs" which contains the desired value.
 * This either puts the value into a register or finds the value in an already
 * existing register.
 */
static inline u8 get_register_with_value(intermediate _intermediate)
{
    /* This puts the desired variable into a valid register. */
    u8 lowest_priority_reg = __UINT8_MAX__;
    for (u32 i=0; i != GENERAL_REGISTER_COUNT; i++) {
        if (i == operational_register)
            continue;
 
        if (regs[i].content.type == CONST)
            lowest_priority_reg = i;

        if (regs[i].content.type == VAR_RETURN) {
            lowest_priority_reg = i;
            goto ARMv7_get_register_with_value_put_in_register_label;
        }

        if (regs[i].content.type == VAR_ACCESS
        && _intermediate.type == VAR_ACCESS
        && _intermediate.ptr == regs[i].content.ptr)
            return i;

    }

    if (lowest_priority_reg != __UINT8_MAX__)
        goto ARMv7_get_register_with_value_put_in_register_label;

    /*
     * If there is not empry registers we replace the register with the lowest
     * priority variable.
     */
    u32 lowest_priority = __UINT32_MAX__;
    for (u32 i=0; i != GENERAL_REGISTER_COUNT; i++) {
        u32 current_priority = get_variable_priority((u32)regs[i].content.ptr);
        if (lowest_priority >= current_priority) {
            lowest_priority_reg = i;
            lowest_priority = current_priority;
        }
    }

    save_register(lowest_priority_reg);

    ARMv7_get_register_with_value_put_in_register_label:

    if (_intermediate.type == CONST) {
        #if VOID_PTR_64BIT
        put_const_into_register((i64)_intermediate.ptr, lowest_priority_reg);
        #else
        put_const_into_register(*(i64*)_intermediate.ptr, lowest_priority_reg);
        #endif
        return lowest_priority_reg;
    } else if (_intermediate.type == VAR_ACCESS
    || _intermediate.type == VAR_ASSIGNMENT) {
        variable_symbol* _var = get_variable_symbol("",(u32)_intermediate.ptr);
        ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
        _var->stack_location, false, lowest_priority_reg, false, false));
    }
    return lowest_priority_reg;
}

/*
 * This sets the intermediates in all of the general purpose registers to 0.
 */
void clear_registers()
{
    intermediate _intermediate = { VAR_RETURN, 0 };
    for (u32 i=0; i < GENERAL_REGISTER_COUNT; i++)
        regs[i].content = _intermediate;
}

/*
 * This frees all of the dynamically allocated vectors and items in this file.
 */
void ARMv7_free_all()
{
    free(variable_locations.contents);
    free(variable_priorities.contents);
}
