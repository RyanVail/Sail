/*
 * This holds the intermediate enum and other general functions involving
 * intermediates.
 */
#ifndef BACKEND_INTERMEDIATE_H
#define BACKEND_INTERMEDIATE_H

#include<types.h>
#include<common.h>
#include<main.h>
#include<datastructures/stack.h>
#include<backend/intermediate/symboltable.h>

// TODO: Spell comparsion right!
typedef enum intermediate_type {
    // One operand required
    INC,                    // ++
    DEC,                    // --
    NOT,                    // !
    COMPLEMENT,             // ~
    NEG,                    // -    // This takes the negative of a value.
    // Two operands required
    ADD,                    // +
    SUB,                    // -
    MUL,                    // *
    DIV,                    // /
    AND,                    // &
    XOR,                    // ^
    OR,                     // |
    LSL,                    // <<
    LSR,                    // >>
    MOD,                    // %

    // Comparisons, two operands
    IS_EQUAL,               // =
    NOT_EQUAL,              // !=
    GREATER_THAN,           // >
    GREATER_THAN_EQUAL,     // >=
    LESS_THAN,              // <
    LESS_THAN_EQUAL,        // <=
    EQUAL,                  // =    // This should only be used as an operation
                                    // into "process_operation".

    // Variable intermediates
    VAR_ASSIGNMENT,         // u32 var = 2
    VAR_ACCESS,             // var
    VAR_MEM,                // &var
    // Memory intermediates
    MEM_LOCATION,           // *ptr = 8
    MEM_ACCESS,             // *ptr

    // Program flow intermediates
    IF,                     // if (true)
    LOOP,                   // loop
    END,                    // }
    CONTINUE,               // Continue
    BREAK,                  // Break
    FUNC_CALL,              // func()
    GOTO,                   // goto label

    // Constant intermediate
    CONST,                  // A signed constant the size of (void*)
    CONST_PTR,              // A ptr to a i128 bit constant the size of (void*)

    // More
    FUNC_RETURN,            // This is used as a place holder for the return of
                            // a function call in "operand_stack".
    MEM_RETURN,             // This is used as a place holder for the return of
                            // "MEM_ACCESS".
    COMPARISON_RETURN,      // This is used as a place holder for the result of
                            // a comprasion.
    VAR_RETURN,             // This is used as a place holder for variable
                            // declerations.
    CAST,                   // This is used to cast the top operand into another
                            // type.
    CLEAR_STACK             // ;
} intermediate_type;

// TODO: This could be optimized to use less memory by not having a "ptr" if we
// have an operation operand.
/* struct intermediate - This struct represents one intermediate token
 * @type: This is the type of this intermediate
 * @ptr: This is either a constant or a ptr to a variable, function, etc. which
 * is determined from the "type".
 */
typedef struct intermediate {
    intermediate_type type;
    void* ptr;
} intermediate;

/* struct operand - This represents an operand on "operand_stack"
 * @intermediate: The "intermediate" token of this operand
 * @inited: If this value is in a register / on the stack yet
 * @type: The type of this operand
 */
typedef struct operand {
    intermediate intermediate;
    bool inited;
    type type;
} operand;

/*
 * This frees "intermediates_vector".
 */
void free_intermediates();

/*
 * This clears the operand stack
 */
void clear_operand_stack();

/*
 * This adds an operand onto the "operand_stack".
 */
void add_operand(intermediate _intermediate, bool inited);

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation);

/*
 * This adds an intermediate to the "intermediates_vector".
 */
void add_intermediate(intermediate _intermediate);

/*
 * This casts the operand on top of "operand_stack" to the desired type.
 */
void cast_top_operand(type _type);

/*
 * This prints the intermediates.
 */
#if DEBUG
void print_intermediates();
#endif

#endif