/*
 * This handles the intermediates of enums.
 */

#ifndef BACKEND_INTERMEDIATE_ENUM_H
#define BACKEND_INTERMEDIATE_ENUM_H

#include<common.h>
#include<datastructures/stack.h>
#include<types.h>
#include<backend/intermediate/typedef.h>

/* struct enum_entry - This represents an entry into an enum
 * @hash: The hash of the name of this entry
 * @value: This value of this entry
 * @parent_enum: The parent enum of this enum entry
 */
typedef struct enum_entry {
    u32 hash;
    i64 value;
    intermediate_typedef* parent_enum;
} enum_entry;

/*
 * This initializes the intermediate enum's hashtable with the equation 
 * (1 << "hash_table_size") for the true size.
 */
void init_enum_hash_table(u8 entry_size);

/* This frees all of the intermediate enums. */
void free_intermediate_enums();

/*
 * This finds and returns the enum entry with the same hash. If no entry is
 * found this returns NULLPTR.
 */
enum_entry* get_enum_entry(u32 hash);

/*
 * This adds the enum entry to the hash table of entries and returns a pointer
 * to the entry. This exits on errors and prints the errors.
 */
enum_entry* add_enum_entry(intermediate_typedef* parent_enum, i64 value, \
char* entry_name);

#endif