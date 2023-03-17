#ifndef INTERMEDIATE_TYPES_H
#define INTERMEDIATE_TYPES_H

#include<common.h>
#include<types.h>

/* This is the last intermediate type that is considered normal. */
#define INTERMEDIATE_TYPE_NORMAL_END (CLEAR_STACK)

// TODO: There should be another intermediate called scope which starts a new
// scope for programming languages that support that.
typedef enum intermediate_type {
    // One operand required
    INC,                    // ++
    DEC,                    // --
    NOT,                    // !
    COMPLEMENT,             // ~
    NEG,                    // -
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
    NOT_EQUAL,              // !=
    IS_EQUAL,               // ==
    GREATER_THAN,           // >
    GREATER_THAN_EQUAL,     // >=
    LESS_THAN,              // <
    LESS_THAN_EQUAL,        // <=
    EQUAL,                  // =

    // Variable intermediates
    VAR_DECLARATION,        // The variable symbol is in the "ptr".
    VAR_ASSIGNMENT,         // The variable hash is in the "ptr".
    VAR_ACCESS,             // The variable hash is in the "ptr".
    VAR_MEM,                // The variable hash is in the "ptr".
    // Memory intermediates
    MEM_LOCATION,           // &value
    MEM_DEREF,              // *ptr

    // Program flow intermediates
    IF,                     // if (true)
    ELSE,                   // else
    LOOP,                   // loop
    END,                    // }
    CONTINUE,               // continue
    RETURN,                 // return
    BREAK,                  // break
    FUNC_DEF,               // fn func() {}
    FUNC_CALL,              // The function hash is in "ptr"
    GOTO,                   // "ptr" is a "char*" to the label name

    // Constant intermediate
    CONST,                  // A signed constant the size of (void*)
    CONST_PTR,              // A ptr to a i64 bit constant the size of (void*)
    FLOAT,                  // The floating point intermediate. The value is
                            // inside of the ptr.
    DOUBLE,                 // The double point intermediate. The value is
                            // inside of the ptr when compiling for 64 bit
                            // platforms. On 32 bit platforms ptr points to the
                            // value.

    // Struct intermediates
    GET_STRUCT_VARIABLE,    // The struct's variable hash is inside of the ptr.

    // More
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