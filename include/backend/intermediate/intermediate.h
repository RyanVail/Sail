/*
 * This holds the intermediate enum and other general functions involving
 * intermediates.
 */
#ifndef BACKEND_INTERMEDIATE_H
#define BACKEND_INTERMEDIATE_H

typedef enum intermediate_type {
    // One operand required
    INC,                    // ++
    DEC,                    // --
    NOT,                    // !
    COMPLEMENT,             // ~
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
    EQUAL,                  // ==
    NOT_EQUAL,              // !=
    GREATER_THAN,           // >
    GREATER_THAN_EQUAL,     // >=
    LESS_THAN,              // <
    LESS_THAN_EQUAL,        // <=

    // Variable intermediates
    VAR_ASSIGNMENT,         // u32 var = 2
    VAR_REASSIGNMENT,       // var = 4
    VAR_ACCESS,             // var
    VAR_MEM,                // &var
    // Memory intermediates
    MEM_ASSIGNMENT,         // *ptr = 8;
    MEM_ACCESS,             // ptr

    // Program flow intermediates
    IF,                     // if (true) {}
    WHILE,                  // while (true) {}
    LOOP,                   // loop {}
    FUNC_CALL,              // func()
    GOTO,                   // goto label

    // Constant intermediate
    CONST,                  // A const signed constant
} intermediate_type;

/* struct intermediate - This struct represents one intermediate token
 * @type: This is the type of this intermediate
 * @ptr: This is either a constant or a ptr to a variable, function, etc. which
 * is determined from the "type".
 */
typedef struct intermediate {
    intermediate_type type;
    void* ptr;
} intermediate;

#endif