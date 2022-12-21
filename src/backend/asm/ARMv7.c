/*
 * This is the ARMv7 assembly / machine code backend.
 *
 * When a variable is used it is put into a register and kept in that register
 * till another variable or a constant takes its place. Variables in registers
 * have the "intermediate_kind" of variable_access. Registers that are being
 * operated on have the "intermediate_kind" of "VAR_RETURN".
 */

#include<backend/asm/ARMv7.h>
#include<backend/intermediate/intermediate.h>

typedef enum location_type {
    STACK_INDEX,
    REG_INDEX,
    IMMEDIATE,
} location_type;

/* struct value_location - This holds the location of a value
 * @first: This is the first half of the location
 * @second: This is the second half of the location, which is only used if the
 * value is 64 bit
 * @type: This is the type of location
 * @value_type: This is the type of value
 */
typedef struct value_location {
    u16 first;
    u16 second;
    location_type type;
    type value_type;
} value_location;

/* struct value_on_stack - This holds the stack location of variables
 * @size: This is the size of the variable, either 8, 32, or 64 bits.
 * @first: This is the first half of the variable
 * @second: This is the second half of the variable
 * @variable_id: This is the variable id that is held in this stack location
 */
typedef struct value_on_stack {
    u8 size;
    u16 first;
    u16 second;
    u32 variable_id;
} value_on_stack;

static bin output_bin = { 0, 0 };
static reg regs[GENERAL_REGISTER_COUNT];
static vector variable_priorities = { 0, 0, 0, 0 };

/*
 * This is the operand stack which just holds the locations of operands and
 * their types.
 */
static stack value_locations = { 0, sizeof(value_location) };
static stack values_on_stack = { 0, sizeof(value_on_stack) };

#define ASSEMBLE_MOVT(condition, destination_reg, _immediate) \
    condition << 28 | 1 << 25 | 1 << 24 | 1 << 22 | destination_reg << 16 \
    | _immediate

#define ASSEMBLE_MOVW(condition, destination_reg, _immediate) \
    condition << 28 | 1 << 25 | 1 << 24 | destination_reg << 16 | _immediate

#define ASSEMBLE_DATA_NO_SHIFT(condition, operation, flags, destination_reg, \
first_reg, second_reg) \
    condition << 28 | operation << 21 | (bool)flags << 20 | first_reg << 16 \
    | destination_reg << 12 | second_reg

#define ASSEMBLE_SWP(condition, byte_or_word, base_reg, destination_reg, \
source_reg) \
    condition << 28 | 2 << 23 | byte_or_word << 22 | base_reg << 16 \
    | destination_reg << 12 | 9 << 4 | source_reg

/*
 * This preforms a ROR on "input" by "shift_count". "shift_count" should be
 * under a value of 32.
 */
#define ROR(input, shift_count) \
    ((u32)input >> (u32)shift_count) + ((u32)input << (32 - (u32)shift_count))

/*
 * This macros gets the rotate needed for "const".
 */
#define GET_ROR_OF_CONST(_const) \
    current_rotate = 0; \
    for (; current_rotate < 18; current_rotate += 2) { \
        if (ROR(_const, current_rotate) < __UINT8_MAX__+1) \
            break; \
    }

/*
 * Offset should be no bigger than a u12. "ldr_or_str" and "byte_or_word" are
 * in the form of "true_or_false" so true in "ldr_or_str" would mean the first
 * value or "ldr".
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
static u32 get_immediate_of_const(u32 _const);

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
}

// TODO: If there is a const in a register with the perfect value a SWP can be
// preformed to get lower memory use and some more speed.
/*
 * This saves the variable inside "reg_index" to the stack. If there is no
 * variable this just returns and does nothing.
 */
static inline void save_register(u8 reg_index)
{
    u32* type_sizes = get_type_sizes;

    if (regs[reg_index].content.type != VAR_ACCESS)
        return;

    variable_symbol* _var = get_variable_symbol("",regs[reg_index].content.ptr);

    ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
    0, true, reg_index, true, false));
}

/*
 * This processes an intermediate token.
 */
static inline void ARMv7_process_intermediate(intermediate _intermediate)
{
    // TODO: I'm pretty sure "_var" and "_first" can be the same variable.
    // "_first" should be defined to the stop of the operand stack at the top
    // instead of repeating the line a bunch of times.
    value_location* _first = 0;
    value_location* _second = 0;
    value_location* _var = 0;
    switch(_intermediate.type)
    {
    case FUNC_DEF:
        clear_variables_in_scope();
        break;
    case CLEAR_STACK:
        while (!(STACK_IS_EMPTY(value_locations)))
            free(stack_pop(&value_locations));
        break;
    case NEG:
        // TODO: This needs to be a reverse subtraction.
        _first = stack_top(&value_locations);
        if (_first->type == REG_INDEX)
            ARMv7_add_asm(14 << 28 | 1 << 25 | 3 << 21 | _first->first << 16 \
            | _first->first << 12 | 1);
    case COMPLEMENT:
        if (_first->type == COMPLEMENT)
            ARMv7_add_asm(14 << 28 | 1 << 25 | 15 << 21 | _first->first << 16 \
            | _first->first << 12 | _first->first);
        break;
    case CONST:
        _var = malloc(sizeof(value_location));
        if (_var == NULLPTR)
            handle_error(0);
        #if VOID_PTR_64BIT
        u32 immediate = get_immediate_of_const((u64)_intermediate.ptr);
        if (immediate != __UINT32_MAX__) {
            _var->type = IMMEDIATE;
            _var->value_type.ptr = 0;
            _var->first = (i64)_intermediate.ptr;
            _var->value_type.kind = get_lowest_type((i64)_intermediate.ptr);
        }
        #else
        u32 immediate = get_immediate_of_const((u64)_intermediate.ptr);
        if (immediate != __UINT32_MAX__) {
            _var->type = IMMEDIATE;
            _var->first = (u32)_intermediate.ptr;
        }
        #endif
        else {
            _var->type = REGISTER;
            _var->first = get_register_with_value(_intermediate);
        }
        stack_push(&value_locations, _var);
        break;
    case LSL:
    case LSR:
        /* The first operand is either an immediate or a register. */
        _first = stack_pop(&value_locations);

        value_location* _second = stack_top(&value_locations);

        if (_first->type == REG_INDEX)
            ARMv7_add_asm(14 << 28 | 13 << 21 | _first->first << 16 \
            | _first->first << 12 | 0 + (_intermediate.type == LSR) * \ 
            (IS_TYPE_NEG(_first->value_type) || \
            IS_TYPE_NEG(_second->value_type)) << 5 \
            | _first->first);
        else if (_first->type == IMMEDIATE)
            ARMv7_add_asm(14 << 28 | 13 << 21 | _first->first << 16 \
            | _first->first << 12 | 0 + (_intermediate.type == LSR) * \
            (IS_TYPE_NEG(_first->value_type) || \
            IS_TYPE_NEG(_second->value_type)) << 8 \
            | _first->first);
        free(_first);
        break;
    case CONST_PTR:
        // TODO: Add constant pointer things here
        break;
    case VAR_DECLERATION:
        add_variable_symbol_ptr((variable_symbol*)_intermediate.ptr);
        break;
    case VAR_ACCESS:
    case VAR_ASSIGNMENT:
        _var = malloc(sizeof(value_location));
        if (_var == NULLPTR)
            handle_error(0);

        _var->value_type = ((variable_symbol*) \
            get_variable_symbol("",(u32)_intermediate.ptr))->type;

        _var->type = REG_INDEX;
        _var->first = get_register_with_value(_intermediate);
        stack_push(&value_locations, _var);
        break;
    case REGISTER:
        variable_priorities = *(vector*)_intermediate.ptr;
        break;
    case NOT:
        _first = stack_top(&value_locations);
        if (_first->type == REG_INDEX) {
            /* CLZ rx, rx */
            ARMv7_add_asm((u32)value_locations.top->value << 12 \
            | _first->first | 0xe16f0f10);
            /* LSL rx, #5 */
            ARMv7_add_asm(14 << 28 | 13 << 21 | _first->first << 16 \
            | _first->first << 12 | 5 << 4 | _first->first);

            regs[_first->first].content.type = BOOL_TYPE;
        }
        break;
    case INC:
    case DEC:
        _first = stack_top(&value_locations);
        if (_first->type == REG_INDEX)
            ARMv7_add_asm(14 << 28 | 1 << 25 \
            | 3 + (_intermediate.type == INC) << 21 | _first->first << 16 \
            | _first->first << 12 | 1);
        break;
    case CAST:
        _first = stack_top(&value_locations);
        #if VOID_PTR_64BIT
        _first->value_type = *((type*)&_intermediate.ptr);
        #else
        _first->value_type = *((type*)_intermediate.ptr);
        #endif
    }

    if ((_intermediate.type >= ADD) && (_intermediate.type <= OR)) {
        // printf("%u\n", (*(value_location*)stack_top(&value_locations)).type);
        value_location* _first = stack_pop(&value_locations);
        value_location* _second = stack_top(&value_locations);
        u8 operational_codes[] = { 4,2,0,0,0,1,12 };
        if (_first->type == REG_INDEX)
            ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, \
            (operational_codes[((u8)_intermediate.type - ADD)]), false, \
            _first->first, _first->first, _second->first));
        else if (_first->type == IMMEDIATE) {
            ARMv7_add_asm(14 << 28 | true << 25 \
            | operational_codes[((u8)_intermediate.type - ADD)] << 21 \
            | _second->first << 16 | _second->first << 12 | \
            get_immediate_of_const(_first->first));
        }
        free(_first);
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
 * This returns the 12 bit immediate of the inputed 32 bit constant.
 */
static u32 get_immediate_of_const(u32 _const)
{
    u8 current_rotate = 0;
    GET_ROR_OF_CONST(_const);
    if (current_rotate == 18)
        return __UINT32_MAX__;
    return current_rotate << 8 | ROR(_const, (32 - current_rotate));
}

/*
 * This puts the inputed constant into the desired register.
 */
static inline void put_u32_const_into_register(u32 _const, u8 _reg)
{
    // TODO: This doesn't clear the register before MOVW
    if (_const < 65536)
        ARMv7_add_asm(ASSEMBLE_MOVW(14, _reg, _const));

    bool is_complement = false;

    u32 immediate_result = __UINT32_MAX__;

    // printf("Put %u into %u\n", _const, _reg);

    if ((_const < __UINT32_MAX__)) {
        is_complement = true;
        _const = ~_const;
        immediate_result = get_immediate_of_const(_const);
    }

    if (immediate_result == __UINT32_MAX__) {
        ARMv7_add_asm(ASSEMBLE_MOVT(14, _reg, (_const >> 16)));
        ARMv7_add_asm(ASSEMBLE_MOVW(14, _reg, ((_const << 16) >> 16)));
        return;
    }

    ARMv7_add_asm(14 << 28 | true << 25 | 15 << 21 | _reg << 15 | _reg << 12 | \
    immediate_result);
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
        // if (i == operational_register)
            // continue;
 
        if (regs[i].content.type == CONST) {
            lowest_priority_reg = i;
            if (_intermediate.type == CONST
            && _intermediate.ptr == regs[i].content.ptr)
                return i;
        }

        #if VOID_PTR_64BIT
        if (regs[i].content.type == CONST_PTR) {
            lowest_priority_reg = i;
            if (_intermediate.type == CONST_PTR
            && *(i64*)_intermediate.ptr == *(i64*)regs[i].content.ptr)
                return i;
        }
        #endif

        if (regs[i].content.type == VAR_RETURN) {
            lowest_priority_reg = i;
            goto ARMv7_get_register_with_value_put_in_register_label;
        }

        if (regs[i].content.type == VAR_ACCESS
        && (_intermediate.type == VAR_ACCESS
        || _intermediate.type == VAR_ASSIGNMENT)
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

    if (regs[lowest_priority].content.type == VAR_ACCESS
    && (_intermediate.type == VAR_ACCESS
    || _intermediate.type == VAR_ASSIGNMENT)) {
        //
    }

    save_register(lowest_priority_reg);

    ARMv7_get_register_with_value_put_in_register_label:

    regs[lowest_priority_reg].content = _intermediate;

    if (_intermediate.type == CONST) {
        #if VOID_PTR_64BIT
        put_u32_const_into_register((u32)_intermediate.ptr, lowest_priority_reg);
        #else
        put_u32_const_into_register(*(u32*)_intermediate.ptr, lowest_priority_reg);
        #endif
        return lowest_priority_reg;
    } else if (_intermediate.type == VAR_ACCESS
    || _intermediate.type == VAR_ASSIGNMENT) {
        variable_symbol* _var = get_variable_symbol("",(u32)_intermediate.ptr);
        ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
        0, false, lowest_priority_reg, false, false));
    }
    return lowest_priority_reg;
}

/*
 * This sets the intermediates in all of the general purpose registers to 0.
 */
void clear_registers()
{
    intermediate _intermediate = { VAR_RETURN, 0 };
    for (u32 i=0; i < GENERAL_REGISTER_COUNT; i++) {
        regs[i].content = _intermediate;
    }
}

/*
 * This frees all of the dynamically allocated vectors and items in this file.
 */
void ARMv7_free_all()
{
    while (!(STACK_IS_EMPTY(value_locations)))
        free(stack_pop(&value_locations));
}

#undef ASSEMBLE_MOVT
#undef ASSEMBLE_MOVW
#undef ASSEMBLE_DATA_NO_SHIFT
#undef ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE
#undef ROR
#undef GET_ROR_OF_CONST