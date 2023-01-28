/*
 * This optimization pass goes through intermediates counting up variable
 * uses between control flow operators and adding REGISTER intermediates.
 * basically splitting the intermediates into basic blocks.
 */

#include<intermediate/optimization/registerpass.h>
#include<intermediate/intermediate.h>
#if DEBUG && linux
#include<time.h>
#endif

static inline void goto_start_of_next_basic_block(u32* location, \
vector* intermediates, vector* output_intermediates);

static inline void process_basic_block(vector* ptrs, vector* uses, u32 start, \
u32 end, vector* intermediates, vector* output_intermediates);

/*
 * This function goes through intermediates counting variable uses between
 * control flow operators and adds "REGISTER" intermediates. This also counts
 * variable uses in the symbol table.
 */
void optimization_do_register_pass()
{
    /*
     * This goes between basic blocks which lack control flow and do
     * optimizations on those blocks. If we assign a value to a location in
     * memory in the basic block this doesn't remove reassignments because that
     * risks breaking undefined behaviour. If the memory location of the
     * variable is requested the flag "needs_stack" is set on the variable
     * symbol.
     */

    #if DEBUG && linux
        clock_t starting_time = clock();
    #endif

    vector* intermediates = get_intermediate_vector();

    vector output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates->size);

    vector ptrs = vector_init_with(sizeof(u32), 4);
    vector uses = vector_init_with(sizeof(u8), 4);

    u32 location = 0;
    u32 start = 0;

    intermediate* start_scope_ptr = 0;

    // TODO: The below if statments should be replaced with a switch statment so
    // the compiler can decide how to optimize it.
    for (; location < VECTOR_SIZE((*intermediates));) {
        intermediate* _current = vector_at(intermediates, location, 0);

        if ((_current->type >= IF) && (_current->type <= GOTO)) {
            process_basic_block(&ptrs, &uses, start, location, \
                intermediates, &output_intermediates);

            goto_start_of_next_basic_block(&location, \
                intermediates, &output_intermediates);

            start = location;
            continue;
        }

        if (_current->type >= VAR_ASSIGNMENT && _current->type <= VAR_MEM) {
            // TODO: This should be reimplemented somehow.
            // get_variable_symbol("", *(u32*)&_current->ptr)->uses++;

            for (u32 i=0; i < VECTOR_SIZE(ptrs); i++) {
                if (*(u32*)vector_at(&ptrs, i, 0) == *(u32*)(&_current->ptr)) {
                    *(u8*)vector_at(&uses, i, 0) += 1;
                    goto optimization_first_pass_next_intermediate_label;
                }
            }
            u8 _tmp_uses = 0;
            vector_append(&ptrs, (u32*)&_current->ptr);
            vector_append(&uses, &(_tmp_uses));
        }

        optimization_first_pass_next_intermediate_label:
        location++;
    }

    free(uses.contents);
    free(ptrs.contents);
    free_intermediates(false, false);
    *get_intermediate_vector() = output_intermediates;

    #if DEBUG && linux
        printf("Took %f ms to do the register pass.\n", \
            (((float)clock() - starting_time) / CLOCKS_PER_SEC) * 1000.0f );
    #endif
}

/*
 * This goes through the variable uses and orders them by the number of uses.
 */
static inline void process_basic_block(vector* ptrs, vector* uses, u32 start, \
u32 end, vector* intermediates, vector* output_intermediates)
{
    if (VECTOR_SIZE((*ptrs)) == 0)
        goto optimization_first_pass_process_basic_block_label;

    /* This reorders ptrs from highest to lowest uses badly. */
    vector* new_ptrs = malloc(sizeof(vector));
    if (new_ptrs == NULLPTR)
        handle_error(0);
    *new_ptrs = vector_init_with(sizeof(u32), ptrs->size);

    // TODO: Replace this with something that isn't O(n^2)!
    for (u8 x=0; x < VECTOR_SIZE((*ptrs)); x++) {
        u8 highest_uses = 0;
        u8 highest_index = __UINT8_MAX__;
        for (u8 y=0; y < VECTOR_SIZE((*ptrs)); y++) {
            if (*(u32*)vector_at(ptrs, y, false) != __UINT32_MAX__
            && *(u8*)vector_at(uses, y, false) >= highest_uses) {
                highest_index = y;
                highest_uses = *(u8*)vector_at(uses, y, false);
            }
        }

        if (highest_index == __UINT8_MAX__)
            continue;

        vector_append(new_ptrs, vector_at(ptrs, highest_index, false));
        *(u32*)vector_at(ptrs, highest_index, false) = __UINT32_MAX__;
    }

    intermediate _intermediate = { REGISTER, new_ptrs };
    vector_append(output_intermediates, &_intermediate);
    ptrs->apparent_size = 0;
    uses->apparent_size = 0;

    optimization_first_pass_process_basic_block_label:
    // TODO: Using "memcpy" instead of appending can increase the speed of this.
    for (; start < end; start++)
        vector_append(output_intermediates, vector_at(intermediates,start,0));
}

/*
 * This incraments "location" adding all control flow operators till it reaches
 * a token that isn't a control flow operator.
 */
static inline void goto_start_of_next_basic_block(u32* location, \
vector* intermediates, vector* output_intermediates)
{
    for (; (*location) < VECTOR_SIZE((*intermediates)); *location += 1) {
        intermediate* _current = vector_at(intermediates, *location, 0);
        if (_current->type < IF || _current->type > GOTO)
            return;
        vector_append(output_intermediates, _current);
    }
}