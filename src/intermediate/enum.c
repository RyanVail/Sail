/*
 * This handles the intermediates of enums.
 */
// TODO: This needs to hold the names of the enums so taking enums as function
// inputs and type casting to enums works correctly. Enum entries should hold a
// ptr to their parents instead of their type.

#include<intermediate/enum.h>
#include<frontend/common/parser.h>
#include<datastructures/hash_table.h>

/* This frees all of the intermediate enums. */
void free_enum_entries(hash_table* _enums)
{
    #if DEBUG
    if (_enums->contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif
}

/*
 * This finds and returns the enum entry with the same hash. If no entry is
 * found this returns NULLPTR.
 */
enum_entry* get_enum_entry(hash_table* _enums, u32 hash)
{
    #if DEBUG
    if (_enums->contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif

    hash_table_bucket* _bucket = hash_table_at_hash(_enums, hash);
    return _bucket == NULLPTR ? NULLPTR : _bucket->value;
}

/*
 * This adds the enum entry to the hash table of entries and returns a pointer
 * to the entry. This returns NULLPTR on errors.
 */
enum_entry* add_enum_entry(intermediate_pass* _pass, \
intermediate_typedef* parent_enum, i64 value, char* entry_name)
{
    hash_table* _enums = &_pass->enums;

    #if DEBUG
    if (_enums->contents == NULLPTR)
        send_error("Intermediate enums have not been initted yet");
    #endif

    /* Hash the variable name. */
    HASH_STRING(entry_name);

    /* Check if the enum name is valid. */
    if (is_invalid_name(_pass, entry_name)) {
        // TODO: Errors in files that aren't the front ends and linker are not
        // a good idea.
        printf("\x1b[091mERROR:\x1b[0m The enum entry name: \"%s\" is invalid.\n",\
            entry_name);
        exit(-1);
    }

    /* Check if the enum entry name has already been used. */
    if (get_enum_entry(_enums, result_hash) != NULLPTR)
        return NULLPTR;

    // TODO: This is doing this logic twice, once in "get_enum_entry" and here.
    hash_table_bucket* _bucket = hash_table_insert_hash(_enums, result_hash);

    /* Putting the enum entry into a bucket. */
    _bucket->value = malloc(sizeof(enum_entry));
    enum_entry* _entry = _bucket->value;
    CHECK_MALLOC(_entry);
    _entry->value = value;
    _entry->hash = result_hash;
    _entry->parent_enum = parent_enum;
    return _entry;
}

/* This clears all of the enums. */
void clear_intermediate_enums(hash_table* _enums)
{
    hash_table_bucket* current_bucket = _enums->contents;
    hash_table_bucket* linked_bucket = NULLPTR;
    for (u32 i=0; i < (1 << _enums->size); i++) {
        if (current_bucket->next != NULLPTR) {
            linked_bucket = current_bucket->next;
            do {
                void* tmp = linked_bucket->next;
                free(linked_bucket);
                linked_bucket = tmp;
            } while (linked_bucket != NULLPTR);
        }
        current_bucket->value = NULLPTR;
        current_bucket->next = NULLPTR;
        current_bucket->hash = 0;
        current_bucket++;
    }
}

/*
 * This frees all of the intermediate enums freeing the typedef parents needs
 * to be done seperatly.
 */
void free_intermediate_enums(hash_table* _enums)
{
    clear_intermediate_enums(_enums);
    free(_enums->contents);
}