#ifndef FRONTEND_SALMON_STRUCT_H
#define FRONTEND_SALMON_STRUCT_H

#include<intermediate/struct.h>
#include<common.h>

// TODO: Document
typedef struct static_member {
    char* name;
    void* value;
    type type;
    u32 hash;
} static_member;

typedef struct salmon_struct_data {
    stack statics;
} salmon_struct_data;

/*
 * This adds a static member to the inputted salmon struct's data created from
 * the inputted "var_type" and "var_name". Returns true if it was successful
 * otherwise false.
 */
bool salmon_add_static_member_to_struct(salmon_struct_data* data, type var_type,
char* var_name, bool override);

#endif