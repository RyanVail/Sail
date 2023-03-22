#ifndef INTERMEDIATE_TYPES_H
#define INTERMEDIATE_TYPES_H

#include<common.h>
#include<types.h>

/* This is the last intermediate type that is considered normal. */
#define INTERMEDIATE_TYPE_NORMAL_END (CLEAR_STACK)

// TODO: These comments should be removed and explained clearly in
// "documentation/intermediate".intermediate.
// TODO: This use of hashes inside ptrs is use less it should just be ptrs to
// the values of them and the hashes really aren't that useful after the first
// front end pass.
// TODO: There should be another intermediate called scope which starts a new
// scope for programming languages that support that.
// TODO: Why is NOT so high up and not in the bitwise??
// TODO: VAR_MEM should be replaced with only MEM_LOCATION.
typedef enum intermediate_type {
    /* Single operand */
    INC,                    // ++ This will just incrament the last thing on
							// the stack it will not incrament the value of a
							// variable.
    DEC,                    // -- This will just decrament the last thing on
							// the stack it will not decrament the value of a
							// variable.
    NOT,                    // !
    COMPLEMENT,             // ~
    NEG,                    // -
    /* Dual operands */
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
    /* Comparisons, dual operands */
    NOT_EQUAL,              // !=
    IS_EQUAL,               // ==
    GREATER_THAN,           // >
    GREATER_THAN_EQUAL,     // >=
    LESS_THAN,              // <
    LESS_THAN_EQUAL,        // <=
    EQUAL,                  // =
    /* Variable intermediates */
    VAR_DECLARATION,        // The variable symbol is in the "ptr".
    VAR_ASSIGNMENT,         // The variable hash is in the "ptr".
    VAR_ACCESS,             // The variable hash is in the "ptr".
    VAR_MEM,                // The variable hash is in the "ptr".
    /* Memory intermediates */
    MEM_LOCATION,           // &value
    MEM_DEREF,              // *ptr
    /* Program flow intermediates */
    IF,                     // if (true)
    ELSE,                   // else
    LOOP,                   // loop
    END,                    // }
    CONTINUE,               // continue
    // TODO: RETURN and BREAK should be switched to make more sense but it might
    // break things.
    RETURN,                 // return
    BREAK,                  // break
    FUNC_DEF,               // fn func() {}
    FUNC_CALL,              // The function hash is in "ptr"
    GOTO,                   // "ptr" is a "char*" to the label name
    /* Constant intermediates */
    CONST,                  // A signed constant the size of (void*)
    CONST_PTR,              // A ptr to a i64 bit constant the size of (void*)
    FLOAT,                  // The floating point intermediate. The value is
                            // inside of the ptr.
    DOUBLE,                 // The double point intermediate. The value is
                            // inside of the ptr when compiling for 64 bit
                            // platforms. On 32 bit platforms ptr points to the
                            // value.
	// TODO: I don't know if this should even be here
    /* Struct intermediate */
    GET_STRUCT_VARIABLE,    // The struct's variable hash is inside of the ptr.
	/* Temp return intermediates */
    FUNC_RETURN,            // This is used as a place holder for the return of
                            // a function call in "operand_stack".
    MEM_RETURN,             // This is used as a place holder for the return of
                            // "MEM_DEREF".
    COMPARISON_RETURN,      // This is used as a place holder for the result of
                            // a comparison.
    VAR_RETURN,             // This is used as a place holder for a variable
                            // declarations.
    CAST,                   // This is used to cast the top operand into another
                            // type. The type is in "ptr".
	// TODO: These should be named something like "VARS_TO_REGISTER" or
	// something like that
	/* Variable optimization intermediates */
    REGISTER,               // This is used to tell the compiler which variables
                            // should be in registers within a basic block. This
                            // contains a pointer to a vector which contains in
                            // order the highest priority variables for
                            // register substitution.
    IGNORE,                 // This is used to tell the compiler which variables
                            // aren't going to be used again within the current
                            // scope. The variable hash is inside the
                            // intermediate ptr.
    VAR_USE,                // This tells the compile what variables are used
                            // inside a scope. This contains a pointer to a
                            // vector of variable ids.
	// TODO: There should be more stack operational intermediates like this and
	// this should be in a section of intermediates that make more sense.
    CLEAR_STACK,            // ;
    NIL,                    // This intermediate is skipped.
} intermediate_type;

/* struct intermediate - This struct represents one intermediate token
 * @type: This is the kind of intermediate this is
 * @ptr: This is either a constant or a ptr to a variable, function, etc. which
 * is determined from the "type".
 */
typedef struct intermediate {
    intermediate_type type;
    void* ptr;
} intermediate;

/* struct operand - This represents an operand on "operand_stack"
 * @intermediate: The "intermediate" token of this operand
 * @initted: If this value is in a register / on the stack yet
 * @type: The type of this operand
 */
typedef struct operand {
    intermediate intermediate;
    bool initted;
    type type;
} operand;

#endif

