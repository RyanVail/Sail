/*
 * This is the ARMv7 assembly / machine code backend.
 *
 * When a variable is used it is put into a register and kept in that register
 * till another variable or a constant takes its place. Variables in registers
 * have the "intermediate_kind" of "VARIABLE_ACCESS". Registers that had
 * variables in them and that are now being operated on have the
 * "intermediate_kind" of "VAR_RETURN". Variables that are being defined and
 * come from "VAR_DECLARATION" keep the "intermediate_type" of "VAR_DECLARATION"
 * till they are defined.
 */

// TODO: The reg struct should be renamed into something like "reg_content" and
// reg should be a typedef of a u8.
// TODO: This doesn't support branches longer than the 24 bit maximum in one
// instruction.

#include<backend/ARMv7.h>
#include<intermediate/intermediate.h>
#include<intermediate/struct.h>

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

/* struct control_flow - This is a simple struct that represnets a control flow
 * operator.
 * @intermediate: The type of control flow operator
 * @offset: The instruction offset of this control flow in bytes
 * @instruction: This is the instruction that defines this control flow "IF"
 * intermediates point to their branch instruction
 */
typedef struct control_flow {
    intermediate_type intermediate;
    u32 offset;
    u32* instruction;
} control_flow;

static bin output_bin = { NULLPTR, 0 };
static reg regs[GENERAL_REGISTER_COUNT];
static vector variable_priorities = {
    .contents = NULLPTR,
    .apparent_size = 0,
    .size = 0,
    .type_size = 0,
};

/*
 * This is the operand stack which just holds the locations of operands and
 * their types.
 */
static stack value_locations = {
    .top = NULLPTR,
};

static stack values_on_stack = {
    .top = NULLPTR,
};

static stack control_flow_stack = {
    .top = NULLPTR,
};

static u32 current_offset = 0;

#define ASSEMBLE_MOVT(condition, destination_reg, _immediate) \
    condition << 28 | 1 << 25 | 1 << 24 | 1 << 22 | _immediate >> 12 << 16 \
    | (u8)destination_reg << 12 | _immediate & 0xfff

#define ASSEMBLE_MOVW(condition, destination_reg, _immediate) \
    condition << 28 | 1 << 25 | 1 << 24 | _immediate >> 12  << 16\
    | (u8)destination_reg << 12 | _immediate & 0xfff

#define ASSEMBLE_DATA_NO_SHIFT(condition, operation, flags, destination_reg, \
first_reg, second_reg) \
    condition << 28 | operation << 21 | (bool)flags << 20 | first_reg << 16 \
    | destination_reg << 12 | second_reg

#define ASSEMBLE_SWP(condition, byte_or_word, base_reg, destination_reg, \
source_reg) \
    condition << 28 | 2 << 23 | byte_or_word << 22 | base_reg << 16 \
    | destination_reg << 12 | 9 << 4 | source_reg

/*
 * This converts a signed offset into a signed 25bit offset to be used in a
 * branch.
 */
#define ASSEMBLE_24BIT_OFFSET(offset) \
    offset < 0 ? (~offset + 1) ^ ((1 << 24) - 1) : offset;

/*
 * This preforms a ROR on "input" by "shift_count". "shift_count" should be
 * under a value of 32.
 */
#define ROR(input, shift_count) \
    ((u32)input >> (u32)shift_count) + ((u32)input << (32 - (u32)shift_count))

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

void ARMv7_clear_registers();
static inline void ARMv7_process_intermediate(intermediate _intermediate);
static inline void ARMv7_add_asm(u32 machine_code_to_add);
static u32 ARMv7_get_immediate_of_const(u32 _const);
static inline u8 ARMv7_get_register_with_value(intermediate _intermediate);

/*
 * This returns a "bin" which contains the outputed ARMv7 machine code generated
 * from the inputted intermediates.
 */
bin ARMv7_intermediates_into_binary(vector* intermediates)
{
    ARMv7_clear_registers();
    output_bin.contents.type_size = sizeof(u32);
    for (u32 i=0; i < VECTOR_SIZE(*intermediates); i++) {
        ARMv7_process_intermediate(VECTOR_AT(intermediates, i, intermediate));
    }
    return output_bin;
}

/*
 * This adds the inputted machine code without a label to "output_bin".
 */
static inline void ARMv7_add_asm(u32 machine_code_to_add)
{
    vector_append(&output_bin.contents, &machine_code_to_add);
    current_offset += 4;
}

/*
 * This saves the variable inside "reg_index" to the stack. If there is no
 * variable this just returns and does nothing.
 */
static inline void ARMv7_save_register(u8 reg_index)
{
    if (regs[reg_index].content.type != VAR_ACCESS)
        return;

    variable_symbol* _var = get_variable_symbol("", \
        (u32)(size_t)regs[reg_index].content.ptr);

    ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
    0, true, reg_index, true, false));
}

/*
 * This processes an intermediate token.
 */
static inline void ARMv7_process_intermediate(intermediate _intermediate)
{
    // TODO: I'm pretty sure "_var" and "_first" can be the same variable. Same
    // "_first" should be defined to the stop of the operand stack at the top
    // instead of repeating the line a bunch of times.
    value_location* _first = 0;
    value_location* _second = 0;
    value_location* _var = 0;
    control_flow* new_control_flow;
    control_flow* end_control_flow;
    // TODO: These should be in order
    switch(_intermediate.type)
    {
    case IF: // BNE IF_END_LABEL / ELSE_START_LABEL
        ARMv7_add_asm(1 << 28 | 5 << 25);
    case LOOP:
        new_control_flow = malloc(sizeof(control_flow));
        CHECK_MALLOC(new_control_flow);

        new_control_flow->intermediate = _intermediate.type;
        new_control_flow->offset = current_offset;
        new_control_flow->instruction = VECTOR_END(&output_bin.contents, u32*);

        stack_push(&control_flow_stack, new_control_flow);
        break;
    case BREAK:
    case RETURN:
        break;
    case ELSE:
        // TODO: End of "IF" before and "ELSE" should branch to the end of the
        // else if we go through the "IF".
    case END:
        // TODO: This shouldn't be as nested as it is.
        end_control_flow = NULLPTR;
        if (!STACK_IS_EMPTY(control_flow_stack)) {
            end_control_flow = stack_pop(&control_flow_stack);
            if (end_control_flow != NULLPTR) {
                i32 new_offset = end_control_flow->offset - current_offset;
                new_offset >>= 2;
                new_offset = ASSEMBLE_24BIT_OFFSET(new_offset);
                if (end_control_flow->intermediate == LOOP)
                    ARMv7_add_asm(14 << 28 | 5 << 25 | new_offset);
                if (end_control_flow->intermediate == IF)
                    *end_control_flow->instruction |= new_offset;
            }
        }
        break;
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
        CHECK_MALLOC(_var);
        #if PTRS_ARE_64BIT
        u32 immediate = ARMv7_get_immediate_of_const((u64)_intermediate.ptr);
        if (immediate != UINT32_MAX) {
            _var->type = IMMEDIATE;
            _var->value_type.ptr = 0;
            _var->first = (i64)_intermediate.ptr;
            _var->value_type.kind = get_lowest_type((i64)_intermediate.ptr);
        }
        #else
        u32 immediate = ARMv7_get_immediate_of_const((u64)_intermediate.ptr);
        if (immediate != UINT32_MAX) {
            _var->type = IMMEDIATE;
            _var->first = (u32)_intermediate.ptr;
        }
        #endif
        else {
            _var->type = REGISTER;
            _var->first = ARMv7_get_register_with_value(_intermediate);
        }
        stack_push(&value_locations, _var);
        break;
    case LSL:
    case LSR:
        /* The first operand is either an immediate or a register. */
        _first = stack_pop(&value_locations);
        _second = stack_top(&value_locations);

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
    case VAR_DECLARATION:
        add_variable_symbol_ptr((variable_symbol*)_intermediate.ptr);
    case VAR_ACCESS:
    case VAR_ASSIGNMENT:
        _var = malloc(sizeof(value_location));
        CHECK_MALLOC(_var)

        _var->value_type = _intermediate.type == VAR_DECLARATION ? \
        ((variable_symbol*)_intermediate.ptr)->type : ((variable_symbol*) \
        get_variable_symbol("",(u32)(size_t)_intermediate.ptr))->type;

        _var->type = REG_INDEX;

        _var->first = ARMv7_get_register_with_value(_intermediate);

        stack_push(&value_locations, _var);
        break;
    case REGISTER:
        variable_priorities = *(vector*)_intermediate.ptr;
        break;
    case EQUAL:
        _first = stack_pop(&value_locations);
        _second = stack_top(&value_locations);
        // TODO: This logic still needs to be added
        printf("%u\n", _first->first);
        if (_second->type == REG_INDEX
        && regs[_second->first].content.type == VAR_DECLARATION)
            regs[_second->first].content.type = VAR_ASSIGNMENT;
        printf("%u\n", regs[_second->first].content.type);
        break;
    case NOT:
        _first = stack_top(&value_locations);
        if (_first->type == REG_INDEX) {
            /* CLZ rx, rx */
            ARMv7_add_asm((u32)(size_t)value_locations.top->value << 12 \
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
        #if PTRS_ARE_64BIT
        _first->value_type = *(type*)&_intermediate.ptr;
        #else
        _first->value_type = *(type*)_intermediate.ptr;
        #endif
    case IS_EQUAL:
    case NOT_EQUAL:
    case GREATER_THAN:
    case GREATER_THAN_EQUAL:
    case LESS_THAN:
    case LESS_THAN_EQUAL:
        _first = stack_pop(&value_locations);
        _second = stack_top(&value_locations);
        u8 operational_codes[] = { 1, 0, 12, 10, 11, 13 };

        /*
         * This is an ugly way of getting the immediate value into a register.
         */
        if (_second->type == IMMEDIATE) {
            intermediate tmp_intermediate = {
                .type = CONST,
                .ptr = (void*)(size_t)_second->first,
            };

            _second->first = ARMv7_get_register_with_value(tmp_intermediate);
        }

        // CMP <first register> <second register>
        ARMv7_add_asm(14 << 28 | 1 << 24 | 1 << 22 | 1 << 20 \
        | _first->first << 16 | _second->first);

        // MOV<operation_code> #1
        ARMv7_add_asm(_second->first << 12 | 0x03a00001 \
        | operational_codes[_intermediate.type - NOT_EQUAL] << 28);
        // MOV<!operation_code> #0
        ARMv7_add_asm(_second->first << 12 | 0x03a00000 \
        | operational_codes[_intermediate.type - NOT_EQUAL + 1] << 28);

        free(_first);
        break;
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case AND:
    case XOR:
    case OR:
        _first = stack_pop(&value_locations);
        _second = stack_top(&value_locations);
        // TODO: This logic should be put into a switch statements so the
        // compiler can optimize it how it likes and it would output the same
        // thing anyway on most computation levels.
        u8 _operational_codes[] = { 4, 2, 0, 0, 0, 1, 12 };
        if (_second->type == REG_INDEX)
            ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, \
            (_operational_codes[((u8)_intermediate.type - ADD)]), false, \
            _second->first, _second->first, _first->first));
        else if (_second->type == IMMEDIATE) {
            ARMv7_add_asm(14 << 28 | 1 << 25 \
            | _operational_codes[((u8)_intermediate.type - ADD)] << 21 \
            | _first->first << 16 | _first->first << 12 | \
            ARMv7_get_immediate_of_const(_second->first));
        }
        free(_first);
        break;
    }
}

/*
 * This returns the priority of the inputted variable id.
 */
static inline u32 ARMv7_get_variable_priority(u32 variable_id)
{
    for (u32 i=0; i < VECTOR_SIZE(variable_priorities); i++)
        if (VECTOR_AT(&variable_priorities, i, u32) == variable_id)
            return i;

    return UINT32_MAX;
}

// TODO: Why is this a static
/*
 * This returns the 12 bit immediate of the inputted 32 bit constant. UINT32_MAX
 * is returned if the inputted constant is invalid.
 */
static u32 ARMv7_get_immediate_of_const(u32 _const)
{
    u8 current_rotate = 0;
    for (; current_rotate < 34; current_rotate += 2) {
        if (ROR(_const, current_rotate) < UINT8_MAX)
            break;
    }

    if (current_rotate == 32)
        return UINT32_MAX;

    return ((32-current_rotate & 0x1f) >> 1) << 8 | ROR(_const, current_rotate);
}

/*
 * This puts the inputted constant into the desired register.
 */
static inline void ARMv7_put_u32_const_into_register(u32 _const, u8 _reg)
{
    // TODO: This doesn't clear the register before MOVW

    u32 immediate = ARMv7_get_immediate_of_const(_const);
    if (immediate != UINT32_MAX) {
        ARMv7_add_asm(14 << 28 | 1 << 25 | 13 << 21 | _reg << 16 | _reg << 12 \
        | immediate);
        return;
    }

    if (_const < UINT16_MAX) {
        ARMv7_add_asm(ASSEMBLE_MOVW(14, _reg, _const));
        ARMv7_add_asm(ASSEMBLE_MOVT(14, _reg, 0));
        return;
    }

    bool is_complement = false;

    u32 immediate_result = UINT32_MAX;

    // Find out why this if statement is here I added it for some reason then
    // removed it and things broke.
    if ((_const <  UINT16_MAX)) {
        is_complement = true;
        _const = ~_const;
        immediate_result = ARMv7_get_immediate_of_const(_const);
    }

    if (immediate_result == UINT32_MAX) {
        ARMv7_add_asm(ASSEMBLE_MOVT(14, _reg, (_const >> 16)));
        ARMv7_add_asm(ASSEMBLE_MOVW(14, _reg, ((_const << 16) >> 16)));
        return;
    }

    ARMv7_add_asm(14 << 28 | true << 25 | 15 << 21 | _reg << 15 | _reg << 12 | \
    immediate_result);
}

/*
 * This copies the inputted variable into the destination reg.
 */
static inline void ARMv7_copy_variable(u32 _var_hash, u8 _reg)
{
    variable_symbol* _var = get_variable_symbol("", _var_hash);

    u8 base_reg = 0;
    // TODO: I'm pretty sure this shouldn't be checking for "VAR_ACCESS"
    // intermediates.
    for (u32 base_reg=0; base_reg < GENERAL_REGISTER_COUNT; base_reg++)
        if ((regs[base_reg].content.type == VAR_ACCESS
        || regs[base_reg].content.type == VAR_ASSIGNMENT)
        && (size_t)regs[base_reg].content.ptr == (size_t)_var_hash)
            break;

    // TODO: This currently only works with variables that are inside of
    // registers.
    #if DEBUG
    if (!(regs[base_reg].content.type == VAR_ACCESS
    || regs[base_reg].content.type == VAR_ASSIGNMENT)
    && (size_t)regs[base_reg].content.ptr != (size_t)_var_hash) {
        printf("Variable hash: %08x\n", _var_hash);
        send_error("Variable to copy is not inside of a register");
    }
    #endif

    /* MOV VAR_REG, _reg */
    ARMv7_add_asm(ASSEMBLE_DATA_NO_SHIFT(14, 13, 0, _reg, _reg, base_reg));
}

// TODO: Variables can be swapped if they're the same size to use less memory.
// TODO: This doesn't account for half words.
/*
 * This returns the register index into "regs" which contains the desired value.
 * This either puts the value into a register or finds the value in an already
 * existing register. The "VAR ACCESS" intermediate causees the variable to be
 * copied into another register.
 */
static inline u8 ARMv7_get_register_with_value(intermediate _intermediate)
{
    /* This puts the desired variable into a valid register. */
    u8 lowest_priority_reg = UINT8_MAX;
    for (u32 i=0; i != GENERAL_REGISTER_COUNT; i++) {
        // if (i == operational_register)
            // continue;
 
        if (regs[i].content.type == CONST) {
            lowest_priority_reg = i;
            if (_intermediate.type == CONST
            && _intermediate.ptr == regs[i].content.ptr)
                return i;
        }

        #if PTRS_ARE_64BIT
        if (regs[i].content.type == CONST_PTR) {
            lowest_priority_reg = i;
            if (_intermediate.type == CONST_PTR
            && *(i64*)_intermediate.ptr == *(i64*)regs[i].content.ptr)
                return i;
        }
        #endif

        if (regs[i].content.type == VAR_RETURN) {
            lowest_priority_reg = i;
            goto ARMv7_ARMv7_get_register_with_value_put_in_register_label;
        }

        if (regs[i].content.type == VAR_ACCESS
        && (_intermediate.type == VAR_ACCESS
        || _intermediate.type == VAR_ASSIGNMENT)
        && _intermediate.ptr == regs[i].content.ptr
        && _intermediate.type != VAR_ACCESS)
            return i;
    }

    if (lowest_priority_reg != UINT8_MAX)
        goto ARMv7_ARMv7_get_register_with_value_put_in_register_label;

    /*
     * If there is not empty registers we replace the register with the lowest
     * priority variable.
     */
    u32 lowest_priority = UINT32_MAX;
    for (u32 i=0; i != GENERAL_REGISTER_COUNT; i++) {
        /* Copying shouldn't replace the base reigster. */
        if (regs[i].content.ptr == _intermediate.ptr)
            continue;

        u32 current_priority = \
            ARMv7_get_variable_priority((u32)(size_t)regs[i].content.ptr);

        if (lowest_priority >= current_priority) {
            lowest_priority_reg = i;
            lowest_priority = current_priority;
        }
    }

    // if (regs[lowest_priority].content.type == VAR_ACCESS
    // && (_intermediate.type == VAR_ACCESS
    // || _intermediate.type == VAR_ASSIGNMENT)) {
        // TODO: I don't know what this if statement was meant to do.
    // }

    ARMv7_save_register(lowest_priority_reg);

    ARMv7_ARMv7_get_register_with_value_put_in_register_label:

    regs[lowest_priority_reg].content = _intermediate;

    variable_symbol* _var;
    switch(_intermediate.type)
    {
    case CONST:
        ARMv7_put_u32_const_into_register((u32)(size_t)_intermediate.ptr, \
            lowest_priority_reg);
        return lowest_priority_reg;
        break;
    case VAR_ASSIGNMENT:
        _var = get_variable_symbol("",(u32)(size_t)_intermediate.ptr);
        ARMv7_add_asm(ASSEMBLE_MEM_PRE_OFFSET_IMMEDIATE(14, true, \
        0, false, lowest_priority_reg, false, false));
        break;
    case VAR_ACCESS:
        ARMv7_copy_variable((u32)(size_t)_intermediate.ptr, lowest_priority_reg);
        break;
    }

    return lowest_priority_reg;
}

/*
 * This takes in a pointer to a struct and generates its padding and memory
 * layout.
 */
void ARMv7_generate_struct(intermediate_struct* _struct)
{
    // TODO: This should try and optimize the padding.
	/* Making a copy of the struct's variables. */
    stack old_stack = _struct->contents;
    _struct->contents.top = NULLPTR;

    /* Copying over the variables with padding in needed places. */
    u32* type_sizes = get_type_sizes();
    u32 allignment = 0;
    u32 remainder;
    u32 type_size;
    struct_variable* _padding;
    struct_variable* _var;

    while (stack_size(&old_stack) != 0) {
        _var = stack_pop(&old_stack);

        /* Adding "type_size" to the allignment and getting the remainder. */
        if (IS_TYPE_STRUCT((_var->type)) && _var->type.ptr == _struct->hash
        && _var->type.kind >> 16 == 0)
            send_error("Structs cannot contain themselves");

        type_size = get_size_of_type(_var->type, &ARMv7_generate_struct);
        allignment += type_size;

        /*
         * If this is a struct (ptrs excluded) that has a size larger than that
         * of a ptr, it will be set to a pointer.
         */
        if (IS_TYPE_STRUCT((_var->type)) && _var->type.kind >> 16 == 0
        && type_size > type_sizes[12])
            type_size = type_sizes[12];

        /* The first step of geting the padding size needed. */
        remainder = allignment % type_size;

        /* The second step of getting the padding size. */
        if (remainder != 0) {
            remainder = type_size - remainder;

            /* Creating and adding the padding. */
            _padding = generate_padding_struct_variable(remainder);
            stack_push(&_struct->contents, _padding);
            allignment += remainder;
        }

        /* Adding the original variable to the stack. */
        stack_push(&_struct->contents, _var);
    }

    _struct->byte_size = allignment;
    reverse_struct_variables(_struct);
}

/*
 * This sets the intermediates in all of the general purpose registers to 0.
 */
void ARMv7_clear_registers()
{
    intermediate _intermediate = {
        .type = VAR_RETURN,
        .ptr = NULLPTR,
    };

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
