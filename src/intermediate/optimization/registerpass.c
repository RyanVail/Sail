/*
 * This optimization pass goes through intermediates counting up variable
 * uses between control flow operators and adding REGISTER intermediates.
 * basically splitting the intermediates into basic blocks.
 */

#include<intermediate/optimization/registerpass.h>
#include<intermediate/intermediate.h>

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

    START_PROFILING("do the register optimization pass", \
    "do all optimization passes");

    vector* intermediates = get_intermediate_vector();

    vector output_intermediates = \
        vector_init(sizeof(intermediate), intermediates->size);

    vector ptrs = vector_init(sizeof(u32), 4);
    vector uses = vector_init(sizeof(u8), 4);

    u32 location = 0;
    u32 start = 0;

    intermediate* start_scope_ptr = 0;

    // TODO: The below if statements should be replaced with a switch statement
    // so the compiler can decide how to optimize it.
    for (; location < VECTOR_SIZE(*intermediates);) {
        intermediate* _current = &VECTOR_AT (
            intermediates,
            location,
            intermediate
        );

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
                if (VECTOR_AT(&ptrs, i, u32) == (u32)(size_t)_current->ptr) {
                    VECTOR_AT(&uses, i, u8) += 1;
                    goto optimization_first_pass_next_intermediate_label;
                }
            }
            u8 _tmp_uses = 0;
            vector_append(&ptrs, (u32*)&_current->ptr);
            vector_append(&uses, &_tmp_uses);
        }

        optimization_first_pass_next_intermediate_label:
        location++;
    }

    END_PROFILING("do the register optimization pass", true);

    free(uses.contents);
    free(ptrs.contents);
    free_intermediates(false, false, false);
    *intermediates = output_intermediates;
}

/*
 * This goes through the variable uses and orders them by the number of uses.
 */
static inline void process_basic_block(vector* ptrs, vector* uses, u32 start, \
u32 end, vector* intermediates, vector* output_intermediates)
{
    if (VECTOR_SIZE(*ptrs) == 0)
        goto optimization_first_pass_process_basic_block_label;

    /* This reorders ptrs from highest to lowest uses badly. */
    vector* new_ptrs = malloc(sizeof(vector));
    CHECK_MALLOC(new_ptrs);
    *new_ptrs = vector_init(sizeof(u32), ptrs->size);

    // TODO: Replace this with something that isn't O(n^2)!
    for (u8 x=0; x < VECTOR_SIZE(*ptrs); x++) {
        u8 highest_uses = 0;
        u8 highest_index = UINT8_MAX;
        for (u8 y=0; y < VECTOR_SIZE(*ptrs); y++) {
            if (VECTOR_AT(ptrs, y, u32) != UINT32_MAX
            && VECTOR_AT(uses, y, u8) >= highest_uses) {
                highest_index = y;
                highest_uses = VECTOR_AT(uses, y, u8);
            }
        }

        if (highest_index == UINT8_MAX)
            continue;

        vector_append(new_ptrs, VECTOR_AT(ptrs, highest_index, u32**));
        VECTOR_AT(ptrs, highest_index, u32) = UINT32_MAX;
    }

    intermediate _intermediate = {
        .type = REGISTER,
        .ptr = new_ptrs,
    };

    vector_append(output_intermediates, &_intermediate);
    ptrs->apparent_size = 0;
    uses->apparent_size = 0;

    optimization_first_pass_process_basic_block_label:

    for (; start < end; start++)
        vector_append(output_intermediates, vector_at(intermediates,start,0));
}

/*
 * This increments "location" adding all control flow operators till it reaches
 * a token that isn't a control flow operator.
 */
static inline void goto_start_of_next_basic_block(u32* location, \
vector* intermediates, vector* output_intermediates)
{
    for (; (*location) < VECTOR_SIZE(*intermediates); *location += 1) {
        intermediate* _current = vector_at(intermediates, *location, 0);
        if (_current->type < IF || _current->type > GOTO)
            return;
        vector_append(output_intermediates, _current);
    }
}