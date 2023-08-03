/*
 * This is the level between the intermediates and the machine code / assembly
 * generation level. rtl instructions are general instructions that are then
 * translated into machine dependent instructions based on the target. During
 * the rtl stage the "operand_stack" in "intermediate_pass" is a stack of
 * "operand"s.
 */

#ifndef BACKEND_RTL_RTL_H
#define BACKEND_RTL_RTL_H

/*
 * The "USE_RTL_OPERAND" flag will define an rtl version of the operand struct
 * that includes the rtl register index the operand is located in.
 */
#ifdef USE_RTL_OPERAND
#define OPERAND_DEFINED

#include<intermediate/types.h>

/* struct operand - This represents an operand in an rtl instruction
 * @immediate_or_reg: If the value of this operand is an immediate or register
 * index true means immediate, reg means register index
 * @content: This is the content of the operand goto rtl instructions will
 * contain the goto intermediate and the ptr in the intermediate will point to
 * the instruction that should be jumped to
 * @type: This is the type of this operand+
 */
typedef struct operand {
    bool immediate_or_reg;
    // TODO: It might be better to have some of these unions be anyanomus.
    union content {
        intermediate immediate;
        u32 reg;
    } content;
    type type;
} operand;

#else
#include<intermediate/types.h>
#endif


#include<intermediate/intermediate.h>
#include<intermediate/pass.h>
#include<datastructures/vector.h>

// TODO: There should be a global table that is set before multi threading is
// started that can inform front ends on the target arch for extra processing
// during front end passes.

/* struct reg - This holds the status of a single register
 * @content: This is the content of this register in intermediate form
 * @type: The type of the content of this register
 */
typedef struct reg {
    intermediate content;
    type type;
} reg;

/* This is the value of immediate_or_reg that means immediate. */
#define RTL_OPERAND_IMMEDIATE (true)

/* This is the value of immediate_or_reg that means reg. */
#define RTL_OPERAND_REG (false)

// TODO: It might be possible to make this and intermediate type a u8.
// TODO: If intermediate_type changes this will have to change too.
/*
 * These are the types of the rtl instructions. As documented in documented in
 * "documentation/rtl.md".
 */
typedef enum rtl_instruction_type {
    /* Directly taken from the intermediates */
    RTL_INC,
    RTL_DEC,
    RTL_NOT,
    RTL_COMPLEMENT,
    RTL_NEG,
    RTL_ADD,
    RTL_SUB,
    RTL_MUL,
    RTL_DIV,
    RTL_AND,
    RTL_XOR,
    RTL_OR,
    RTL_LSL,
    RTL_LSR,
    RTL_MOD,
    RTL_NOT_EQUAL,
    RTL_IS_EQUAL,
    RTL_GREATER_THAN,
    RTL_GREATER_THAN_EQUAL,
    RTL_LESS_THAN,
    RTL_LESS_THAN_EQUAL,
    /* Actual assembly instructions */
    RTL_MOV,
    RTL_ROR,
    RTL_GOTO,
    RTL_LOAD,
    RTL_STORE,
    RTL_NIL,
    // TODO: For the latter rtl passes it might be better to have stack_inc and
    // stack_dec instructions.
} rtl_instruction_type;

/* struct rtl - This represents a single rtl instruction
 * @destination: The register that the result is put into
 * @type: The type of the instruction
 * @operand0: The first operand of the instruction
 * @operand1: The second operand of the instruction
 */
typedef struct rtl_instruction {
    operand operand0;
    operand operand1;
    u32 destination;
    rtl_instruction_type type;
} rtl_instruction;

/* This translates and returns the inputted intermediates in rtl form. */
intermediate_pass intermediates_into_rtl(vector* intermediates);

/* This clears all of the defined rtl labels and regs. */
void clear_rtl(intermediate_pass* _pass);

#endif