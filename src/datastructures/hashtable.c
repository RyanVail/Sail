/*
 * This is a very primitive hashtable implementation using a really simple hash
 * function.
 */


#include<datastructures/hashtable.h>

/*
 * This inserts a hash into the hash table and returns a pointer to the
 * hash table bucket so the value can be inserted.
 */
hash_table_bucket* hash_table_insert_hash(hash_table* _hash_table, u32 hash)
{
    /* Checking if the hash has already been defined. */
    #if DEBUG
    if (hash_table_at_hash(_hash_table,  hash) != NULLPTR)
        send_error("Hash already found.\n");
    #endif

    /* Getting the index of the first bucket. */
    hash_table_bucket* current_bucket = _hash_table->contents \
        +sizeof(hash_table_bucket) * (hash & ((1 << _hash_table->size) - 1));

    /* Inserting the current value into the bucket. */
    if (current_bucket->hash == 0) {
        current_bucket->hash = hash;
        return current_bucket;
    }

    hash_table_bucket* new_bucket = malloc(sizeof(hash_table_bucket));
    if (new_bucket == NULLPTR)
        exit(-1);

    *new_bucket = *current_bucket;

    current_bucket->hash = hash;
    current_bucket->next = new_bucket;

    return current_bucket;
}

/*
 * This inserts a string hash into the hash table and returns a pointer to the
 * hash table bucket so the value can be inserted.
 */
hash_table_bucket* hash_table_insert_string(hash_table* _hash_table, \
char* _string)
{
    HASH_STRING(_string);
    return hash_table_insert_hash(_hash_table, result_hash);
}

/*
 * This returns a pointer to the bucket bound to the hash. If no bucket is found
 * NULL is returned.
 */
hash_table_bucket* hash_table_at_hash(hash_table* _hash_table, u32 hash)
{
    /* Getting the index of the bucket. */
    #if WIN32
    hash_table_bucket* current_bucket = (void*)_hash_table->contents \
        (u64)(+sizeof(hash_table_bucket)*(hash & ((1 <<_hash_table->size)-1)));
    #else
    hash_table_bucket* current_bucket = _hash_table->contents \
        + sizeof(hash_table_bucket) * (hash & ((1 <<_hash_table->size) - 1));
    #endif

    /* Getting the value from the bucket. */
    while (true)
    {
        if (current_bucket->hash == hash)
            return current_bucket;

        if (current_bucket->next == NULLPTR)
            return NULL;

        current_bucket = current_bucket->next;
    }
}

/*
 * This returns a pointer to the bucket bound to the same hash as the inputed
 * string. If no bucket is found NULL is returned.
 */
hash_table_bucket* hash_table_at_string(hash_table* _hash_table, char* _string)
{
    HASH_STRING(_string);
    return hash_table_at_hash(_hash_table, result_hash);
}

/*
 * This initalizes and returns the hash table.
 */
hash_table hash_table_init(u8 size)
{
    hash_table_bucket* new_hash_table_contents = \
        malloc(sizeof(hash_table_bucket) * (1 << size));
    if (new_hash_table_contents == NULLPTR)
        handle_error(0);

    for (u32 i=0; i < (1 << size); i++) {
        new_hash_table_contents[i].next = 0;
        new_hash_table_contents[i].hash = 0;
    }

    hash_table new_hash_table = { size, (void*)new_hash_table_contents };
    return new_hash_table;
}
