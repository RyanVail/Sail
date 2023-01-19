/*
 * This holds a lot of functions that apply to the intermediate repersentation
 * of structs.
 */

// TODO: This should hold the names of the structs and the names of their
// variables.

#ifndef INTERMEDIATE_STRUCT_H
#define INTERMEDIATE_STRUCT_H

#include<common.h>
#include<datastructures/stack.h>
#include<datastructures/hashtable.h>
#include<types.h>

/* struct struct_variable - This represents a variable in a struct
 * @type: The type of this variable in the struct
 * @hash: The hash of this variables name
 * @name: The name of this variable
 */
typedef struct struct_variable {
    type type;
    u32 hash;
    char* name;
} struct_variable;

/* struct intermediate_struct - The intermediate repersentation of a struct
 * @hash: The hash of the name of this struct
 * @name: The name of this struct
 * @contents: The variables this struct contains a vector of "struct_variable"
 * @flags: 1 -> optimize padding
 */
typedef struct intermediate_struct {
    u32 hash;
    char* name;
    stack contents;
    u8 flags;
} intermediate_struct;

/* This initializes the intermediate structs hashtable with the inputed size. */
void init_struct_hash_tables(u8 hash_table_size);

/* This frees all of the intermediate structs. */
void free_intermediate_structs();

/* This returns a pointer to the struct with the same hashed name. */
intermediate_struct* find_struct(u32 struct_hash);

/*
 * This function attemps the create a struct with "struct_name". Returns a
 * pointer to the newely created struct otherwise NULLPTR.
 */
intermediate_struct* create_struct(u8 flags, char* struct_name);

/*
 * This finds and returns the "struct_variable" of the inputed variable. Returns
 * NULLPTR if nothing is found.
 */
struct_variable* get_variable_from_struct(u32 struct_hash, char* var_name);

/*
 * This adds the inputed variable to the inputed struct. Returns true if it was
 * successful otherwise false.
 */
bool add_variable_to_struct(intermediate_struct* _struct, type var_type, \
char* var_name);

#endif