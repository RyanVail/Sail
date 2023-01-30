/*
 * This optimization pass goes through all of the intermediates and evalutes
 * arithments invloving constants.
 */

#include<intermediate/optimization/constantpass.h>
#include<evaluate.h>
#include<types.h>
#if DEBUG && linux
#include<time.h>
#endif

// TODO: "MUL_DIV" should be used.
typedef enum communativity {
    NONE,
    NEGATION,
    ADD_SUB,
    MUL_DIV,
} communativity;

/*
 * This goes through all of the the constants in the intermediates and evalutes
 * them. Constants as in values inside of "CONST" / "CONSTPTR" intermediates.
 * This also remvoes/simplifies things like double negatives. 
 */
void optimization_do_constant_pass()
{
    #if DEBUG && linux
        clock_t starting_time = clock();
    #endif

    #if !VOID_PTR_64BIT
        i64 operand0;
        i64 operand1;
        intermediate result_intermediate;
        i64* _result;
    #endif

    /*
     * The communativity type of the last operator. Also will be set to "UKNOWN"
     * if an operand isn't a constant.
     */
    communativity last_communativity = NONE;

    /* The second to last constant. */
    intermediate* last_last_constant = NULLPTR;

    /* The actual last constant. */
    intermediate* last_constant = NULLPTR;

    // TODO: This should be getting the lowest possible type of the constants.
    /* The result type is defaulted to a "u32" unless there's a cast. */
    type result_type = { 0, U32_TYPE };

    i64 result;
    u32 location = 0;
    vector* intermediates = get_intermediate_vector();
    vector output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates->size);

    for (; location < VECTOR_SIZE((*intermediates)); location++) {
        intermediate* _current = vector_at(intermediates, location, false);

        switch(_current->type)
        {
        #if !VOID_PTR_64BIT
        case CONST_PTR:
        #endif
        case CONST:
            last_last_constant = last_constant;

            /*
             * Setting the last constant location to it's to be new location in
             * the output intermediates.
             */
            last_constant = vector_at(&output_intermediates, \
                VECTOR_SIZE(output_intermediates), true);
            break;
        case ADD:
        case SUB:
            if ((last_communativity == ADD_SUB || last_communativity == NONE)
            && last_constant != NULLPTR
            && last_last_constant != NULLPTR) {
                /* Evaluating the constant expression. */
                #if VOID_PTR_64BIT
                result = evaluate_expression(last_last_constant->ptr, \
                    last_constant->ptr, _current->type);
                #else
                /* Freeing the operands after getting them. */
                if (last_last_constant->type == CONST_PTR) {
                    operand0 = *((i64*)last_last_constant->ptr);
                    free(last_last_constant->ptr);
                    last_last_constant->ptr = NULLPTR;
                } else {
                    operand0 = last_last_constant->ptr;
                }

                if (last_constant->type == CONST_PTR) {
                    operand1 = *((i64*)last_constant->ptr);
                    free(last_constant->ptr);
                    last_constant->ptr = NULLPTR;
                } else {
                    operand1 = last_constant->ptr;
                }

                result = evaluate_expression(operand0,operand1,_current->type);
                #endif

                // TODO: For ptrs this needs to do something different.
                /* Scaling the result. */
                result = scale_value_to_type(result, result_type);

                /* Popping the last two constants. */
                vector_pop(&output_intermediates);
                vector_pop(&output_intermediates);

                /* Moving the constants. */
                last_last_constant = NULLPTR;

                // TODO: This logic is copy pasted from "add_const_num". There
                // should some way to share the logic.

                /* Adding the result back to the intermediates. */
                #if !VOID_PTR_64BIT
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
                _current->type = CONST;
                _current->ptr = (void*)result;
                #endif
            }
            last_communativity = ADD_SUB;
            break;
        case MUL:
        case DIV:
            last_communativity = MUL_DIV;
            break;
        case VAR_ACCESS:
            break;
        case CAST:
            #if VOID_PTR_64BIT
            result_type = *(type*)(&_current->ptr);
            #else
            result_type = *((type*)_current->ptr);
            #endif
            break;
        default:
            result_type.kind = U32_TYPE;
            result_type.ptr = 0;
            last_communativity = NONE;
            last_constant, last_last_constant = NULLPTR;
            break;
        }
        vector_append(&output_intermediates, _current);
    }
    free_intermediates(false, false, false);
    *intermediates = output_intermediates;

    #if DEBUG && linux
        printf("Took %f ms to do the constant pass.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f );
    #endif
}