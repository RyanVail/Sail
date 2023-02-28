/*
 * This holds the intermediate enum and other general functions involving
 * intermediates.
 */
#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include<types.h>
#include<common.h>
#include<main.h>
#include<datastructures/stack.h>
#include<intermediate/symboltable.h>

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
    VAR_RETURN,             // This is used as a place holder for variable
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

/*
 * This takes either one or two operands off of "operand_stack" based on "dual"
 * then checks if the operand types are valid for an operation. If comparison
 * both values get taken off.
 */
void pop_operand(bool dual, bool comparison);

/*
 * This frees "intermediates_vector". The symbol table should be in a clean
 * scope before this function is called.
 */
void free_intermediates(bool free_variable_symbols, bool free_var_vectors, \
bool free_constants);

/*
 * This clears the operand stack
 */
void clear_operand_stack();

/*
 * This adds an operand onto the "operand_stack" from an intermediate.
 */
void add_operand(intermediate _intermediate, bool initted);

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
 * This returns a pointer to the intermediate vector.
 */
vector* get_intermediate_vector();

/* This adds the inputted f32 onto the operand stack. */
void add_float(f32 value);

/* This adds the inputted f64 onto the operand stack. */
void add_double(f64 value);

/*
 * This puts the const num into the inputted "_intermediate" be that as a
 * "CONST" intermediate or a "CONST_PTR" based on the "const_num".
 */
void set_intermediate_to_const(intermediate* _intermediate, i64 const_num);

/*
 * This adds the inputted constant number to the operand stack. This will
 * convert it into a "CONST_PTR" if it can't fit into a pointer but it's a
 * "CONST" by default.
 */
void add_const_num(i64 const_num);

// TODO: This function should be in a common frontend file I don't know which
// right now but it really shouldn't be here.
/*
 * If the ASCII number at the inputted token is valid it adds it to the
 * intermediates. Returns true if a number was added, otherwise false.
 */
bool add_if_ascii_num(char* token);

/*
 * If the inputted token is a valid float or double it adds it to the
 * intermediates. Returns the ending index of the float if something was added,
 * otherwise 0.
 */
u32 add_if_ascii_float(char** starting_token);

/* This returns a pointer to the operand stack. */
stack* get_operand_stack();

/*
 * This prints the intermediates.
 */
#if DEBUG
void print_intermediates();
#endif

#endif