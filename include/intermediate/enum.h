/*
 * This handles the intermediates of enums.
 */

#ifndef INTERMEDIATE_ENUM_H
#define INTERMEDIATE_ENUM_H

#include<common.h>
#include<datastructures/stack.h>
#include<types.h>
#include<intermediate/typedef.h>

/* struct enum_entry - This represents an entry into an enum
 * @hash: The hash of the name of this entry
 * @value: This value of this entry
 * @parent_enum: The parent enum of this enum entry
 */
typedef struct enum_entry {
    u32 hash;
    num value;
    intermediate_typedef* parent_enum;
} enum_entry;

/*
 * This finds and returns the enum entry with the same hash. If no entry is
 * found this returns NULLPTR.
 */
enum_entry* get_enum_entry(hash_table* _enums, u32 hash);

/*
 * This adds the enum entry to the hash table of entries and returns a pointer
 * to the entry. This returns NULLPTR on errors.
 */
enum_entry* add_enum_entry(intermediate_pass* _pass, \
intermediate_typedef* parent_enum, num value, char* entry_name);

/* This clears all of the enums. */
void clear_intermediate_enums(hash_table* _enums);

/*
 * This frees all of the intermediate enums freeing the typedef parents needs
 * to be done seperatly.
 */
void free_intermediate_enums(hash_table* _enums);

#endif