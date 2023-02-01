/*
 * This optimization pass goes through all of the intermediates and evalutes
 * arithments invloving constants.
 */

#include<intermediate/optimization/constantpass.h>
#include<evaluate.h>
#include<types.h>
#if DEBUG && linux
#include<cli.h>
#include<time.h>
#endif

typedef enum communativity {
    NONE,
    NEGATION,
    ADD_SUB,
    MUL_DIV,
} communativity;

void constant_optimization_process_operation(intermediate* operand0, \
intermediate* operand1, intermediate* _current, type result_type);

/*
 * This goes through all of the the constants in the intermediates and evalutes
 * them. Constants as in values inside of "CONST" / "CONSTPTR" intermediates.
 * This also remvoes/simplifies things like double negatives. This pass will
 * leave NILs in the intermediates.
 */
void optimization_do_constant_pass()
{
    #if DEBUG && linux
    clock_t starting_time = clock();
    #endif

    #if !PTRS_ARE_64BIT
    i64 operand0_value;
    i64* operand1_value;
    intermediate result_intermediate;
    i64* _result;
    #endif

    /*
     * The communativity type of the last operator. Also will be set to "UKNOWN"
     * if an operand isn't a constant.
     */
    communativity last_communativity = NONE;

    /* This is the operand stack. This has ptrs to the intermediate vector. */
    stack operand_stack = { NULLPTR, sizeof(intermediate) };

    // TODO: This should be getting the lowest possible type of the constants.
    // Or type definitions hsould be bound to a u32 by default rather than the
    // smallest possible type.

    /* The result type is defaulted to a "u32" unless there's a cast. */
    type result_type = { 0, U32_TYPE };

    u32 location = 0;
    intermediate* operand0;
    intermediate* operand1;
    vector* intermediates = get_intermediate_vector();

    for (; location < VECTOR_SIZE((*intermediates)); location++) {
        intermediate* _current = vector_at(intermediates, location, false);

        switch(_current->type)
        {
        #if !PTRS_ARE_64BIT
        case CONST_PTR:
        #endif
        case VAR_ACCESS:
        case CONST:
            stack_push(&operand_stack, _current);
            break;
        case ADD:
        case SUB:
            // TODO: This needs to pop based on the correct things
            operand0 = stack_pop(&operand_stack);
            operand1 = stack_top(&operand_stack);
            if (last_communativity == ADD_SUB || last_communativity == NONE)
                constant_optimization_process_operation(&operand0, &operand1, \
                    _current, result_type);
            last_communativity = ADD_SUB;
            break;
        case MUL:
        case DIV:
            stack_pop(&operand_stack);
            last_communativity = MUL_DIV;
            break;
        case CAST:
            #if PTRS_ARE_64BIT
            result_type = *(type*)(&_current->ptr);
            #else
            result_type = *((type*)_current->ptr);
            #endif
            break;
            // TODO: A lot of the below statments need an implementation.
        case INC:
        case DEC:
        case COMPLEMENT:
        case NEG:
            break;
        case AND:
        case XOR:
        case OR:
        case LSL:
        case LSR:
        case MOD:
            stack_pop(&operand_stack);
            break;
        case IS_EQUAL:
        case NOT_EQUAL:
        case GREATER_THAN:
        case GREATER_THAN_EQUAL:
        case LESS_THAN:
        case LESS_THAN_EQUAL:
            stack_pop(&operand_stack);
            break;
        case VAR_MEM:
        case MEM_LOCATION:
        case MEM_ACCESS:
            break;
        case GET_STRUCT_VARIABLE:
            break;
        default:
            while (stack_size(&operand_stack) != 0)
                stack_pop(&operand_stack);
            result_type.kind = U32_TYPE;
            result_type.ptr = 0;
            last_communativity = NONE;
            break;
        }
    }

    #if DEBUG && linux
    if (get_global_cli_options()->time_compilation)
        printf("Took %f ms to do the constant pass.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f );
    #endif
}

/* This process an int operation for the constant optimization pass. */
void constant_optimization_process_operation(intermediate* operand0, \
intermediate* operand1, intermediate* _current, type result_type)
{
    i64 result;

    #if PTRS_ARE_64BIT
    if (!(operand0->type == CONST && operand1->type == CONST))
        return;
    #else
    if (!(((operand0->type == CONST || operand0->type == CONST_PTR)
    && (operand1->type == CONST || operand1->type == CONST_PTR))))
        return;
    #endif
    /* Evaluating the constant expression. */
    #if PTRS_ARE_64BIT
    result = evaluate_expression(operand1->ptr, operand0->ptr, \
        _current->type);
    #else
    /* Freeing the operands after getting them. */
    if (*operand0->type == CONST_PTR) {
        *operand0_value = *((i64*)*operand0->ptr);
        free(*operand0->ptr);
    } else {
        *operand0_value = *operand0->ptr;
    }

    if (*operand1->type == CONST_PTR) {
        *operand1_value = (i64*)*operand1->ptr;
        *operand1->ptr = NULLPTR;
    } else {
        *operand1_value = &(*operand1->ptr);
    }

    result = evaluate_expression(*operand1,*operand0,_current->type);
    #endif

    operand0->type = NIL;

    // TODO: For ptrs this needs to do something different.
    /* Scaling the result. */
    result = scale_value_to_type(result, result_type);

    // TODO: This logic is copy pasted from "add_const_num". There
    // should some way to share the logic.

    /* Adding the result back to the intermediates. */
    #if !PTRS_ARE_64BIT
    if (result < ~((i64)1 << ((sizeof(void*) << 3))-1)
    || result > ((i64)1 << ((sizeof(void*) << 3)-1))) {
        _result = malloc(sizeof(i64));
        if (_result == NULLPTR)
            handle_error(0);
        *_result = result;
        _current->type = CONST_PTR;
    } else {
        _result = (void*)result;
        _current->type = CONST;
    }
    _current->ptr = _result;
    #else
    _current->type = NIL;
    operand1->ptr = result;
    _current->ptr = (void*)result;
    #endif
}