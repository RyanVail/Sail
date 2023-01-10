/*
 * This does a pass through the intermediates and groups together scope variable
 * uses.
 */

#include<backend/intermediate/optimization/usescopepass.h>
#include<backend/intermediate/intermediate.h>

void optimizaiton_do_use_scope_pass()
{
    vector intermediates = { 0, 0, 0, sizeof(intermediate) };
    vector output_intermediates = { 0, 0, 0, sizeof(intermediate) };
    intermediate* start_scope_ptr = 0;

    intermediates = *get_intermediate_vector();
    output_intermediates = \
        vector_init_with(sizeof(intermediate), intermediates.size);

    // TODO: This should be a hash table.
    vector scope_uses = vector_init_with(sizeof(u32), 4);

    for (u32 i=0; i < VECTOR_SIZE(intermediates); i++) {
        intermediate* _current = vector_at(&intermediates, i, 0);

        if (_current->type >= VAR_DECLERATION && _current->type <= VAR_MEM) {
            u32 _var_id = 0;
            if (_current->type == VAR_DECLERATION)
                _var_id = ((variable_symbol*)_current->ptr)->hash;
            else
                _var_id = (u32)_current->ptr;

            for (u32 x=0; x < VECTOR_SIZE(scope_uses); x++)
                if (_var_id == *(u32*)vector_at(&scope_uses, x, 0))
                    goto optimizaiton_do_use_scope_pass_var_already_used_label;
            vector_append(&scope_uses, &_var_id);
        }

        optimizaiton_do_use_scope_pass_var_already_used_label:

        vector_append(&output_intermediates, _current);

        if (_current->type == FUNC_DEF || i == 0
        || i == VECTOR_SIZE(intermediates) - 1) {
            if (!i)
                goto optimizaiton_do_use_scope_pass_put_intermediate_label;

            vector* new_scope_uses = malloc(sizeof(vector));
            if (new_scope_uses == NULLPTR)
                handle_error(0);
            *new_scope_uses = vector_init_with(sizeof(u32), scope_uses.size);

            // TODO: This should use "memcpy".
            for (u8 i=0; i < VECTOR_SIZE(scope_uses); i++)
                vector_append(new_scope_uses, vector_at(&scope_uses,i,0));

            scope_uses.apparent_size = 0;

            // TODO: Here and again lower at the "REGISTER" intermediate the new
            // vectors might need to be shrinked because if there is one scope
            // that uses a lot of variables every "REGISTER" intermediate and
            // "VAR_USE" is going to use a lot of memory.

            if (start_scope_ptr)
                start_scope_ptr->ptr = new_scope_uses;

            optimizaiton_do_use_scope_pass_put_intermediate_label: ;

            if (i != VECTOR_SIZE(intermediates) - 1) {
                intermediate _intermediate = { VAR_USE, 0 };
                vector_append(&output_intermediates, &_intermediate);
                start_scope_ptr = vector_at(&output_intermediates, \
                    VECTOR_SIZE(output_intermediates)-1, 0);
            }
        }
    }
    free(scope_uses.contents);
    free_intermediates(false, false);
    *get_intermediate_vector() = output_intermediates;
}