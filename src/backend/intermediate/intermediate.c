/*
 * This holds the intermediate enum and other general functions involving
 * intermediates. Intermediates should be feed in to "add_intermediate" in
 * reverse polish notation.
 */
#include<backend/intermediate/intermediate.h>

static stack operand_stack = { 0, sizeof(operand) };
static vector intermediates_vector = { 0, 0, 0, sizeof(intermediate) };

/*
 * This takes in an operand and adds the intermediates to
 * "intermediate_vector".
 */
void add_operand_to_intermediates(intermediate _intermediate)
{
    /* These intermediates are just placeholders. */
    if (_intermediate.type == FUNC_RETURN || _intermediate.type == MEM_RETURN)
        return;

    vector_append(&intermediates_vector, &_intermediate);
}

/*
 * This takes either one or two operands off of "operand_stack" based on "dual"
 * then checks if the operand types are valid for an operation. If comparison
 * both values get taken off.
 */
void pop_operand(bool dual, bool comparison)
{
    if (stack_size(&operand_stack) < (u32)dual+1)
        send_error("Not enough operands to perform operation");

    operand* __first_operand;

    if (!dual || comparison)
        __first_operand = stack_pop(&operand_stack);
    else
        __first_operand = stack_top(&operand_stack);


    if (!__first_operand->inited)
        add_operand_to_intermediates(__first_operand->intermediate);
    intermediate _first_operand = __first_operand->intermediate;

    if (!dual) {
        if (get_type_of_intermediate(_first_operand).kind == VOID_TYPE) {
            printf("Cannot preform operation on a \'");
            print_type(get_type_of_intermediate(_first_operand));
            printf("\' type");
            send_error("");
        }
        return;
    }

    operand* __second_operand = stack_top(&operand_stack);
    if (!__second_operand->inited)
        add_operand_to_intermediates(__second_operand->intermediate);
    intermediate _second_operand = __second_operand->intermediate;

    if (!type_can_implicitly_cast_to(\
    get_type_of_intermediate(_second_operand),\
    get_type_of_intermediate(_first_operand), true)) {
        printf("CAN\n");
    } else {
        printf("CANNOT\n");
    }
}


/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation)
{
    #if DEBUG
    if (_operation < INC || _operation > LESS_THAN_EQUAL) {
        printf("Got unexpected operation operand: %u", _operation);
        abort();
    }
    #endif
    if (_operation == EQUAL)
        //
    if (_operation == CLEAR_STACK)
        clear_operand_stack();
    if (_operation >= ADD && _operation <= MOD)
        pop_operand(true, false);
    if (_operation >= IS_EQUAL && _operation <= LESS_THAN_EQUAL)
        pop_operand(true, true);
    if (_operation >= INC && _operation <= NEG)
        pop_operand(false, false);
}

/*
 * This adds an intermediate to the "intermediates_vector".
 */
void add_intermediate(intermediate _intermediate)
{
    //
}

/*
 * This function takes in an "intermediate" and returns the type that it would
 * evaluate to.
 */
type get_type_of_intermediate(intermediate _intermediate)
{
    #if DEBUG
    if (_intermediate.type >= INC && _intermediate.type <= LESS_THAN_EQUAL) {
        printf("Cannot get the type of intermediate: %u", _intermediate.type);
        abort();
    }
    #endif
    switch (_intermediate.type) {
        case CONST:
        case CONST_PTR:
            type_kind _kind = get_lowest_type((i128)_intermediate.ptr);
            type _type = { _kind, 0 };
            return _type;
            break;
        case VAR_ASSIGNMENT:
        case VAR_REASSIGNMENT:
        case VAR_ACCESS:
        case VAR_MEM:
            return ((variable_symbol*)_intermediate.ptr)->type;
            break;
        case FUNC_RETURN:
            return ((function_symbol*)_intermediate.ptr)->return_type;
            break;
        case MEM_RETURN:
        case MEM_LOCATION:
            return *((type*)_intermediate.ptr);
            break;
        default:
            type __type = { VOID_TYPE, 0 };
            return __type;

    }
    
}

/*
 * This adds an operand onto the "operand_stack".
 */
void add_operand(intermediate _intermediate)
{
    //
}

/*
 * This clears the operand stack.
 */
void clear_operand_stack()
{
    //
}