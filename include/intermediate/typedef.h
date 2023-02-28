/*
 * This holds a lot of function that apply to the intermedaite representation of
 * typedefs.
 */

#ifndef INTERMEDIATE_TYPEDEF_H
#define INTERMEDIATE_TYPEDEF_H

#include<common.h>
#include<datastructures/hash_table.h>
#include<types.h>

/* struct intermediate_typedef - This is the intermediate representation of a
 * typedef
 * @hash: The hash of the name of this typedef
 * @name: The name of this typedef
 * @type: The type of this typedef
 */
typedef struct intermediate_typedef {
    u32 hash;
    char* name;
    type type;
} intermediate_typedef;

/*
 * This initializes the intermediate enum's hashtable with the equation 
 * (1 << "hash_table_size") for the true size.
 */
void init_typedef_hash_table(u8 size);

/*
 * This finds and returns the typedef with the same hash. If none are found
 * this will return NULLPTR.
 */
intermediate_typedef* get_typedef(u32 hash);

/* This creates a typedef and returns a pointer to it. */
intermediate_typedef* add_typedef(char* typedef_name, type typedef_type);

#endif