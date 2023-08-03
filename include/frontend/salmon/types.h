#ifndef FRONTEND_SALMON_TYPES_H
#define FRONTEND_SALMON_TYPES_H

#include<types.h>

/* This returns true if this abstract type has the abstract struct flag. */
#define ABSTRACT_TYPE_IS_STRUCT(_type) (((_type).flags & ABSTRACT_STRUCT_FLAG) \
== ABSTRACT_STRUCT_FLAG)

/*
 * This returns true if this abstract type has the abstract func flag. The use
 * of "ABSTRACT_STRUCT_FLAG" in the and statment is intentional.
 */
#define ABSTRACT_TYPE_IS_FUNC(_type) (((_type).flags & ABSTRACT_STRUCT_FLAG) \
== ABSTRACT_FUNC_FLAG)

typedef enum _salmon_type_kind {
    ABSTRACT_TYPE = TYPE_KIND_LAST_INTERNAL + 1,
} _salmon_type_kind;

typedef enum salmon_type_flags {
    STATIC_FLAG = 1 << 0,
    CONST_FLAG = 1 << 1,
    MACRO_FLAG = 1 << 2,
} salmon_type_flags;

/* These are type flags specific to the abstract type. */
typedef enum salmon_abstract_type_flags {
    ABSTRACT_STRUCT_FLAG = 3 << 6,
    ABSTRACT_FUNC_FLAG = 2 << 6,
} salmon_abstract_type_flags;

/*
 * This returns true if the two inputted abstract types are interchangable
 * meaning they can overide each other.
 */
bool salmon_type_abstracts_are_equal(type abstract_0, type abstract_1);

/*
 * This returns true if the inputted "override" type can override the original
 * type.
 */
bool salmon_type_can_override(type original, type override);

#endif