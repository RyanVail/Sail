/*
 * This optimization pass goes through all of the intermediates and evalutes
 * arithments invloving constants.
 */

#include<intermediate/optimization/constantpass.h>
#include<frontend/common/evaluate.h>
#include<types.h>
#if DEBUG && linux
#include<time.h>
#endif

// TODO: "MUL_DIV" should be used.
typedef enum communativity {
    UNKNOWN,
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

    /*
     * The communativity type of the last operator. Also will be set to "UKNOWN"
     * if an operand isn't a constant.
     */
    communativity last_communativity;

    intermediate* last_constant = NULLPTR;
    u32 location = 0;
    vector* intermediates = get_intermediate_vector();
    vector output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates->size);

    for (; location < VECTOR_SIZE((*intermediates));) {
        intermediate* _current = vector_at(intermediates, location, false);

        switch(_current->type)
        {
        #if !VOID_PTR_64BIT
        case CONST_PTR:
        #endif
        case CONST:
            last_constant = _current;
            break;
        case ADD:
        case SUB:
            if (last_communativity == ADD_SUB) {
                //
            }
            last_communativity = ADD_SUB;
            break;
        case MUL:
        case DIV:
            last_communativity = MUL_DIV;
            break;
        default:
            last_communativity = UNKNOWN;
            if (last_constant != NULLPTR)
                vector_append(&output_intermediates, last_constant);
            last_constant = NULLPTR;
        }
    }
}