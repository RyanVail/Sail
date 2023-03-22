/*
 * This is the level between the intermediates and the machine code / assembly
 * generation level. RTL instructions are general instructions that are then
 * translated into machine dependent instructions based on the target.
 */

#ifndef BACKEND_RTL_RTL_H
#define BACKEND_RTL_RTL_H

#include<common.h>
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

/* struct rtl_operand - This represents an operand in an rtl instruction
 * @immediate_or_reg: If the value of this operand is an immediate or register
 * index true means immediate, reg means register index
 * @content: This is the content of the operand goto rtl instructions will
 * contain the goto intermediate and the ptr in the intermediate will point to
 * the instruction that should be jumped to
 */
typedef struct rtl_operand {
    bool immediate_or_reg;
    union content {
        intermediate immediate;
        u32 reg;
    } content;
} rtl_operand;

// TODO: It might be possible to make this a u8.
/* These are the types of the rtl instructions. */
typedef enum rtl_instruction_type {
    /* Directly taken from the intermediates. */
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

    /* Actual assembly instructions. */
    RTL_MOV,
    RTL_ROR,
    RTL_GOTO,
    RTL_NIL,                    /* This is just used as a tmp label. */
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
    rtl_operand operand0;
    rtl_operand operand1;
    u32 destination;
    rtl_instruction_type type;
} rtl_instruction;

/* This translates and returns the inputted intermediates in RTL form. */
intermediate_pass intermediates_into_rtl(vector* intermediates);

/* This clears all of the defined RTL labels and regs. */
void clear_rtl();

#endif