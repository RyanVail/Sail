/*
 * This holds a lot of functions that apply to the intermediate representation
 * of structs.
 */

#ifndef INTERMEDIATE_STRUCT_H
#define INTERMEDIATE_STRUCT_H

#include<common.h>
#include<datastructures/stack.h>
#include<datastructures/hash_table.h>
#include<intermediate/struct.h>
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

/* struct intermediate_struct - The intermediate representation of a struct
 * @hash: The hash of the name of this struct
 * @name: The name of this struct
 * @contents: The variables this struct contains. A stack of "struct_variable"
 * @flags: 1 -> Padding can be optimized, 2 -> Memory layout has been generated
 * @byte_size: The size of this struct in bytes __UINT16_MAX__ if not set
 */
typedef struct intermediate_struct {
    u32 hash;
    char* name;
    stack contents;
    u8 flags;
    u16 byte_size;
} intermediate_struct;

/* This inits the intermediate struct's hashtable with the inputted size. */
void init_struct_hash_table(u8 hash_table_size);

/* This frees all of the intermediate structs. */
void free_intermediate_structs();

/* This finds and returns the struct with the same hash as the inputted hash. */
intermediate_struct* get_struct(u32 struct_hash);

/*
 * This function attemps the create a struct with "struct_name". Returns a
 * pointer to the newly created struct. Sends errors if any are encountered.
 */
intermediate_struct* create_struct(u8 flags, char* struct_name);

/*
 * This finds and returns the "struct_variable" variable from the inputted
 * struct ptr. This returns NULLPTR if the variable wasn't found.
 */
struct_variable* get_variable_from_struct_ptr(intermediate_struct* _struct, \
char* var_name);

/*
 * This finds and returns the "struct_variable" of the inputted variable.
 * This returns NULLPTR if the variable wasn't found.
 */
struct_variable* get_variable_from_struct_hash(u32 struct_hash, char* var_name);

/*
 * This adds the inputted variable to the inputted struct. Returns true if it
 * was successful otherwise false.
 */
bool add_variable_to_struct(intermediate_struct* _struct, type var_type, \
char* var_name);

/*
 * This goes though all the variables in a struct and reverses their order. This
 * is used after adding padding as all members of a struct would be reversed
 * otherwise.
 */
void reverse_struct_variables(intermediate_struct* _struct);

/*
 * This returns a pointer to the "intermediate_structs" hashtable.
 */
hash_table* get_intermediate_structs();

/*
 * This function goes through all of the defined struct in the hash table and
 * generates their memory layout and anything else using the inputted
 * "struct_generator" function.
 */
void generate_structs(void struct_generator(intermediate_struct*));

/*
 * This generates a place holder struct variables in the heap and returns a
 * pointer to it. Padding "struct_variable"s have a hash equal to 0, random type
 * values and the name is a "u32" that holds the number of bytes of padding.
 */
struct_variable* generate_padding_struct_variable(u32 bytes_of_padding);

#if DEBUG

/*
 * This function prints all of the variables in the struct. This function is for
 * debugging and is not normally called.
 */
void print_struct(intermediate_struct* _struct);

#endif

#endif