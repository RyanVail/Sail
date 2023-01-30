/*
 * This is a very primitive hashtable implementation using a really simple hash
 * function.
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include<common.h>

/*
 * This expands to a for loop that hashes "_string" and the result is found in
 * newly defined variable "result_hash".
 */
#define HASH_STRING(_string) \
    u32 result_hash = __UINT32_MAX__; \
    for (u32 string_index = 0; _string[string_index] != '\0'; string_index++){\
        result_hash ^= _string[string_index]; \
        result_hash = (result_hash << 6) + (result_hash >> 28); \
    }

struct hash_table_bucket;

/* struct hash_table_bucket - This holds a bucket of a hash table
 * @next: The next bucket value in this linked list
 * @hash: The has of this bucket
 * @value: The value of this bucket
 */
typedef struct hash_table_bucket {
    struct hash_table_bucket* next;
    u32 hash;
    void* value;
} hash_table_bucket;

/* struct hash_table - This holds a hash table
 * @size: This number of buckets in this hash table (1 << size) is the true
 * number of buckets.
 * @contents: The contents of this hash table
 */
typedef struct hash_table {
    u8 size;
    void* contents;
} hash_table;

/*
 * This returns a pointer to the bucket bound to the hash. If no bucket is found
 * NULL is returned.
 */
hash_table_bucket* hash_table_at_hash(hash_table* _hash_table, u32 hash);

/*
 * This returns a pointer to the bucket bound to the same hash as the inputed
 * string. If no bucket is found NULL is returned.
 */
hash_table_bucket* hash_table_at_string(hash_table* _hash_table, char* _string);

/*
 * This inserts a hash into the hash table and returns a pointer to the
 * hash table bucket so the value can be inserted.
 */
hash_table_bucket* hash_table_insert_hash(hash_table* _hash_table, u32 hash);

/*
 * This inserts a string hash into the hash table and returns a pointer to the
 * hash table bucket so the value can be inserted.
 */
hash_table_bucket* hash_table_insert_string(hash_table* _hash_table, \
char* _string);

/*
 * This initalizes and returns the hash table.
 */
hash_table hash_table_init(u8 size);

#endif
