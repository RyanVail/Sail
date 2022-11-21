/*
 * This holds the intermediate enum and other general functions involving
 * intermediates. Intermediates should be feed in to "add_intermediate" in
 * reverse polish notation.
 */
#include<backend/intermediate/intermediate.h>
#include<datastructures/stack.h>
#include<common.h>

static stack operand_stack;

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation)
{

}

void add_intermediate(intermediate_type _intermediate)
{
    //
}