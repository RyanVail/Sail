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
#include<intermediate/types.h>
#include<intermediate/symboltable.h>
#include<intermediate/pass.h>

/*
 * This adds an intermediate to the "intermediates_vector" of the inputted
 * intermediate pass.
 */
static inline void add_intermediate(intermediate_pass* _pass, intermediate \
_intermediate)
{
    vector_append(&_pass->intermediates, &_intermediate);
}

/*
 * This takes in an intermediate and adds the inputted intermediate to the
 * inputted pass' intermediate vector if it's not a temp intermediate return
 * type indicated by _RETURN at the end of the name of the intermediate type.
 */
void add_back_intermediate(intermediate_pass* _pass, intermediate \
_intermediate);

/*
 * This takes either one or two operands off of "operand_stack" based on "dual"
 * then checks if the operand types are valid for an operation. If comparison
 * both values get taken off.
 */
void pop_operand(intermediate_pass* _pass, bool dual, bool comparison);

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_pass* _pass, intermediate_type _operation);

/*
 * This both casts the top operand on the operand stack to the inputted type and
 * adds a type intermediate.
 */
void add_cast_intermediate(intermediate_pass* _pass, type _type);

/*
 * This casts the operand on top of "operand_stack" to the desired type. This
 * doesn't add a cast inermediate use "add_cast_intermediate" for that.
 */
void cast_top_operand(intermediate_pass* _pass, type _type);

/*
 * This takes in an "operand" and sets its "type" to the type of the
 * intermediate it contains.
 */
void set_type_of_operand(intermediate_pass* _pass, operand* _operand);

/*
 * This adds an operand onto the inputted intermediate pass' "operand_stack"
 * from the inputted intermediate.
 */
void add_operand(intermediate_pass* _pass, intermediate _intermediate, \
bool initted);

/* This clears the operand stack from the inputted intermediate pass. */
void clear_operand_stack(intermediate_pass* _pass);

/*
 * This frees the intermediates of the inputted intermediate pass. The symbol
 * table should be in a clean scope before this function is called.
 */
void free_intermediates(intermediate_pass* _pass, bool free_variable_symbols, \
bool free_var_vectors, bool free_constants);

/* This adds the inputted f32 to the inputted pass' operand stack. */
void add_float_intermediate(intermediate_pass* _pass, f32 value);

/* This adds the inputted f64 to the inputted pass' operand stack. */
void add_double_intermediate(intermediate_pass* _pass, f64 value);

/*
 * This puts the const num into the inputted "_intermediate" be that as a
 * "CONST" intermediate or a "CONST_PTR" based on the "const_num" and the ptr
 * size of the computer the compiler is running on.
 */
void set_intermediate_to_const(intermediate* _intermediate, i64 const_num);

/*
 * This adds the inputted constant number to the operand stack of the inputted
 * intermediate pass. This will convert the constant numebr into a "CONST_PTR"
 * if it can't fit into a pointer otherwise it will be a "CONST".
 */
void add_const_num(intermediate_pass* _pass, i64 const_num);

// TODO: This function should be in a common frontend file I don't know which
// right now but it really shouldn't be here.
/*
 * If the ASCII number at the inputted token is valid it adds it to the
 * intermediates of the inputted intermediate pass. Returns true if a number
 * was added, otherwise false.
 */
bool add_if_ascii_num(intermediate_pass* _pass, char* token);

/*
 * If the inputted token is a valid float or double it adds it to the
 * intermediates of the inputted intermediate pass. Returns the ending index of
 * the float if something was added, otherwise 0.
 */
u32 add_if_ascii_float(intermediate_pass* _pass, char** starting_token);

/* This returns true if the inputted intermediate type is temp. */
static inline bool intermediate_type_is_temp_return(intermediate_type _type)
{
    switch (_type)
    {
    case FUNC_RETURN:
    case VAR_RETURN:
    case MEM_RETURN:
    case COMPARISON_RETURN:
        return true;
    }
    return false;
}

#if DEBUG
/* This prints the intermediates. */
void print_intermediates(intermediate_pass* _pass);
#endif

#endif