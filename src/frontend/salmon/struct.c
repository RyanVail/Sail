#include<frontend/salmon/struct.h>
#include<frontend/salmon/types.h>
#include<datastructures/hash_table.h>

// TODO: Document
static_member* salmon_get_static_member_from_struct_data(salmon_struct_data*
data, char* var_name)
{
    HASH_STRING(var_name);
    link* _link = data->statics.top;
    while (true) {
        if (_link == NULLPTR)
            return NULLPTR;
        if (((static_member*)_link->value)->hash == result_hash)
            return (static_member*)_link->value;
        _link = _link->next;
    }
}

/*
 * This adds a static member to the inputted salmon struct's data created from
 * the inputted "var_type" and "var_name". Returns true if it was successful
 * otherwise false.
 */
bool salmon_add_static_member_to_struct(salmon_struct_data* data, type var_type,
char* var_name, bool override)
{
    /* Allocating a copy of the member's name. */
    char* new_var_name = malloc(sizeof(var_name)+1);
    CHECK_MALLOC(new_var_name);
    strcpy(new_var_name, var_name);

    HASH_STRING(new_var_name);
    static_member* existing_member = salmon_get_static_member_from_struct_data (
        data,
        var_name
    );

    if (existing_member == NULLPTR) {
        if (override)
            return false;
    } else {
        if (!override)
            return false;
    }

    static_member* new_member;
    if (!override) {
        new_member = malloc(sizeof(static_member));
        CHECK_MALLOC(new_member);
    } else {
        new_member = existing_member;
    }

    new_member->name = new_var_name;
    new_member->value = NULLPTR;
    new_member->type = var_type;
    new_member->hash = result_hash;

    if (!override)
        stack_push(&data->statics, new_member);
}