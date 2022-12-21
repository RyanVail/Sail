/*
 * This does a pass after the basic block pass and adds ignore intermediates to
 * "REGISTER" intermediates when a variable is not used within a scope anymore.
 */

#include<backend/intermediate/optimization/ignorepass.h>

/*
This uses index for simplicity rather than pointers.
Tracking the uses of variable "c":
|--------------|
|REGISTER|VL RL|        VL = Var last used index
|0| abc  |0   0|        RL = Reister last used index
|1| ab   |0   1|
|2| cb   |2   2|
|3| b    |3   2|
|--------------|
*/

static inline void reset_ignore_indexs(vector* ignore_indexs, u32 i, \
vector* intermediates)
{
    u32 register_count = 0;
    for (; i < VECTOR_SIZE((*intermediates)); i++) {
        intermediate* _intermediate = vector_at(intermediates, i, 0);
        if (_intermediate->type == REGISTER)
            register_count++;
        if (_intermediate->type == FUNC_DEF)
            break;
    }

    if (ignore_indexs->contents != NULLPTR) {
        for (u32 x = 0; x < VECTOR_SIZE((*ignore_indexs)); x++)
            free(((vector*)vector_at(ignore_indexs, x, 0))->contents);
        free(ignore_indexs->contents);
    }

    vector _tmp_vector = { 0, 0, 0, sizeof(u32) };
    vector_append(ignore_indexs, &_tmp_vector);
}

/*
 * This goes down from "i" till is hits another scope and for each "REGISTER"
 * intermediate hit that contains "var_id", a pointer is set to that "REGISTER"
 * intermediate, another pointer is set to the last "REGISTER" intermediate. If
 * the last use of "var_id" isn't the same as the last "REGISTER" the number of
 * "REGISTER" intermediates passed before the last use of "var_id" is returned.
 * Otherwise __UINT32_MAX__ is returned.
 */
static inline u32 try_to_ignore_variable(u32 i, u32 var_id, \
vector* intermediates)
{
    u32 registers_count = 0;
    u32 last_use_index = __UINT32_MAX__;
    for (; i < VECTOR_SIZE((*intermediates)); i++) {
        intermediate* _intermediate = vector_at(intermediates, i, 0);
        if (_intermediate->type == REGISTER) {
            registers_count++;
            vector* _var_vec = _intermediate->ptr;
            u32* _end_var = vector_at(_var_vec, VECTOR_SIZE((*_var_vec)), true);
            for (u32*_var =_var_vec->contents; _var<_end_var; _var+=sizeof(u32))
                if (*_var == var_id)
                    last_use_index = registers_count;
        }

        // TODO: This should also be done on the second to last index of the
        // intermediates.
        if (_intermediate->type == FUNC_DEF) {
            if (last_use_index == __UINT32_MAX__
            || last_use_index == registers_count)
                return __UINT32_MAX__;
            return last_use_index;
        }
    }
}

/*
 * This passes through the intermediates adding "IGNORE" intermediates when
 * variables are no longer used.
 */
void optimization_do_ignore_pass()
{
    vector* intermediates = get_intermediate_vector();
    vector output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates->size);

    vector ignore_indexs = { 0, 0, 0, sizeof(vector) };

    for (u32 i=0; i < VECTOR_SIZE((*intermediates)); i++) {
        intermediate* _current = vector_at(intermediates, i, 0);

        if (_current->type == FUNC_DEF || i == 0)
            reset_ignore_indexs(&ignore_indexs, 0, intermediates);

        if (_current->type != VAR_USE)
            continue;

        u32* _end_var = vector_at((vector*)_current->ptr, \
            VECTOR_SIZE((*(vector*)_current->ptr)), true);

        // TODO: If a variable needs to be in the stack this shouldn't try
        // to ignore it.
        for (u32* _var = ((vector*)_current->ptr)->contents;_var != _end_var;) {
            try_to_ignore_variable(i, *(u32*)_current->ptr, intermediates);
            _var += sizeof(u32);
        }
    }
    *get_intermediate_vector() = output_intermediates;
}