/*
 * This handles the intermediates of enums.
 */
// TODO: This needs to hold the names of the enums so taking enums as function
// inputs and type casting to enums works correctly. Enum entries should hold a
// ptr to their parents instead of their type.

#include<intermediate/enum.h>
#include<frontend/common/parser.h>
#include<datastructures/hash_table.h>

static hash_table enum_entries = { sizeof(enum_entry), NULLPTR };

/*
 * This initializes the intermediate enum's hashtable with the equation 
 * (1 << "hash_table_size") for the true size.
 */
void init_enum_hash_table(u8 entry_size)
{
    enum_entries = hash_table_init(entry_size);
}

/* This frees all of the intermediate enums. */
void free_enum_entries()
{
    #if DEBUG
    if (enum_entries.contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif
}

/*
 * This finds and returns the enum entry with the same hash. If no entry is
 * found this returns NULLPTR.
 */
enum_entry* get_enum_entry(u32 hash)
{
    #if DEBUG
    if (enum_entries.contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif

    hash_table_bucket* _bucket = hash_table_at_hash(&enum_entries, hash);
    return _bucket == NULLPTR ? NULLPTR : _bucket->value;
}

/*
 * This adds the enum entry to the hash table of entries and returns a pointer
 * to the entry. This exits on errors and prints the errors.
 */
enum_entry* add_enum_entry(intermediate_typedef* parent_enum, i64 value, \
char* entry_name)
{
    #if DEBUG
    if (enum_entries.contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif

    /* Hash the variable name. */
    HASH_STRING(entry_name);

    /* Check if the enum name is valid. */
    if (is_invalid_name(entry_name)) {
        printf("\x1b[091mERROR:\x1b[0m The enum entry name: \"%s\" is invalid.\n",\
            entry_name);
        exit(-1);
    }

    /* Check if the enum entry name has already been used. */
    if (get_enum_entry(result_hash) != NULLPTR) {
        printf("\x1b[091mERROR:\x1b[0m The enum entry name: \"%s\" has already been used.\n", entry_name);
        exit(-1);
    }

    // TODO: This is doing this logic twice, once in "get_enum_entry" and here.
    hash_table_bucket* _bucket = hash_table_insert_hash(&enum_entries, \
        result_hash);

    /* Putting the enum entry into a bucket. */
    _bucket->value = malloc(sizeof(enum_entry));
    enum_entry* _entry = _bucket->value;
    CHECK_MALLOC(_entry);
    _entry->value = value;
    _entry->hash = result_hash;
    _entry->parent_enum = parent_enum;
    return _entry;
}