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

    if (dual || comparison)
        __first_operand = stack_pop(&operand_stack);
    else
        __first_operand = stack_top(&operand_stack);

    if (!__first_operand->inited)
        add_operand_to_intermediates(__first_operand->intermediate);
    __first_operand->inited = true;

    if (!dual) {
        if (__first_operand->type.kind == VOID_TYPE) {
            printf("Cannot preform operation on a \'");
            print_type(__first_operand->type);
            printf("\' type");
            send_error("");
        }
        return;
    }

    operand* __second_operand;

    if (comparison)
        __second_operand = stack_pop(&operand_stack);
    else
        __second_operand = stack_top(&operand_stack);

    intermediate _second_operand = __second_operand->intermediate;

    if (__second_operand->intermediate.type == VAR_RETURN)
        goto pop_operand_second_operand_is_place_holder_label;

    if (!__second_operand->inited)
        add_operand_to_intermediates(__second_operand->intermediate);
    __second_operand->inited = true;

    pop_operand_second_operand_is_place_holder_label:

    printf("%u : %u\n", __first_operand->type.kind, __second_operand->type.kind);
    if (type_can_implicitly_cast_to( \
    __first_operand->type, __second_operand->type, false)) {
        printf("CAN\n");
    } else {
        printf("CANNOT\n");
    }

    if (dual)
        free(__first_operand);
    if (comparison) {
        free(__second_operand);
        intermediate _to_add_intermediate = { COMPARISON_RETURN, 0 };
        add_operand(_to_add_intermediate, true);
    }
}

/*
 * This processes an operation by taking the needed variables off the operand
 * stack.
 */
void process_operation(intermediate_type _operation)
{
    #if DEBUG
    if (_operation < INC || _operation > LESS_THAN_EQUAL
    && _operation != EQUAL && _operation != CLEAR_STACK) {
        printf("Got unexpected operation operand: %u\n", _operation);
        exit(-1);
    }
    #endif
    if (_operation == EQUAL)
        pop_operand(true, false);
    if (_operation == CLEAR_STACK)
        clear_operand_stack();
    if (_operation >= ADD && _operation <= MOD)
        pop_operand(true, false);
    if (_operation >= IS_EQUAL && _operation <= LESS_THAN_EQUAL)
        pop_operand(true, true);
    if (_operation >= INC && _operation <= NEG)
        pop_operand(false, false);
    intermediate _intermediate = { _operation, 0 };
    add_intermediate(_intermediate);
}

/*
 * This adds an intermediate to the "intermediates_vector".
 */
void add_intermediate(intermediate _intermediate)
{
    vector_append(&intermediates_vector, &_intermediate);
}

/*
 * This casts the operand on top of "operand_stack" to the desired type.
 */
void cast_top_operand(type _type)
{
    ((operand*)stack_top(&operand_stack))->type = _type;
}

/*
 * This takes in an "operand" and sets its "type" to the type of the
 * intermediate it contains. This should only be called on an operand once.
 */
void set_type_of_operand(operand* _operand)
{
    #if DEBUG
    if (_operand->intermediate.type >= INC
    && _operand->intermediate.type <= LESS_THAN_EQUAL) {
        printf("Cannot get the type of intermediate: %u", \
            _operand->intermediate.type);

        abort();
    }
    #endif
    type _type = { 0, VOID_TYPE };
    switch (_operand->intermediate.type) {
        case CONST_PTR:
            _type.kind = get_lowest_type(*((i128*)_operand->intermediate.ptr));
            _operand->type = _type;
            break;
        case CONST:
            _type.kind = get_lowest_type((i128)_operand->intermediate.ptr);
            _operand->type = _type;
            break;
        case VAR_ASSIGNMENT:
        case VAR_RETURN:
        case VAR_ACCESS:
        case VAR_MEM:
            _operand->type = ((variable_symbol*)_operand->intermediate.ptr)->type;
            break;
        case FUNC_RETURN:
            _operand->type = ((function_symbol*)_operand->intermediate.ptr)->return_type;
            break;
        case MEM_RETURN:
        case MEM_LOCATION:
            _operand->type = *((type*)_operand->intermediate.ptr);
            break;
        default:
            _type.kind = VOID_TYPE;
            _operand->type = _type;
    }
}

/*
 * This adds an operand onto the "operand_stack".
 */
void add_operand(intermediate _intermediate, bool inited)
{
    operand* _operand = malloc(sizeof(operand));
    if (_operand == 0)
        handle_error(0);
    _operand->intermediate = _intermediate;
    _operand->inited = inited;
    set_type_of_operand(_operand);
    stack_push(&operand_stack, _operand);
}

/*
 * This clears the operand stack.
 */
void clear_operand_stack()
{
    while (!(stack_is_empty(operand_stack)))
        free((char*)stack_pop(&operand_stack));
}

/*
 * This frees "intermediates_vector".
 */
void free_intermediates()
{
    while (intermediates_vector.size != 0)
        free(vector_pop(&intermediates_vector));

    free(intermediates_vector.contents);
}

/*
 * This prints the intermediates.
 */
#if DEBUG
// TODO: Replace the bellow array with something less temperary.
const char* INTERMEDIATES_TEXT[] = { "Incrament", "Decrament", "Not", "Complement", "Negate", "Add", "Sub", "Mul", "Div", "And", "Xor", "Or", "LSL", "LSR", "Mod", "==", "!=", ">", ">=", "<", "<=", "=", "Var assignment", "Var access", "Var mem", "Mem location", "Mem access", "If", "Loop", "End", "Continue", "Break", "Func call", "Goto", "Const", "Const_ptr", "Func return", "Mem return", "Comparison return", "Var return", "Cast", "Clear stack"};
void print_intermediates()
{
    for (u32 i=0; i < VECTOR_SIZE(intermediates_vector); i++)
        printf("INTER: %s\n",INTERMEDIATES_TEXT[\
            ((intermediate*)vector_at(&intermediates_vector, i, false))->type]);
}
#endif