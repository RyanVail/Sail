#include<frontend/salmon/types.h>

/*
 * This returns true if the two inputted abstract types are interchangable
 * meaning they can overide each other.
 */
bool salmon_type_abstracts_are_equal(type abstract_0, type abstract_1)
{
    return (abstract_0.flags == abstract_1.flags);
}

/*
 * This returns true if the inputted "override" type can override the original
 * type.
 */
bool salmon_type_can_override(type original, type override)
{
    /* Checking the normal salmon flags. */
    if (original.flags & 7 != override.flags & 7)
        return false;

    /* If this is overriding an abstract type. */
    if (original.kind == ABSTRACT_TYPE) {
        if (override.kind == FUNC_TYPE
        && ABSTRACT_TYPE_IS_FUNC(original))
            return true;

        if (override.kind == STRUCT_TYPE
        && ABSTRACT_TYPE_IS_STRUCT(original))
            return true;
    }

    /* Pointers can override pointers. */
    if (original.ptr_count != 0 && override.ptr_count != 0)
        return true;

    return false;
}