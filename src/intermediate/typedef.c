/*
 * This holds a lot of function that apply to the intermediate representation of
 * typedefs.
 */

#include<intermediate/typedef.h>
#include<frontend/common/parser.h>
#include<intermediate/pass.h>

/*
 * This finds and returns the typedef with the same hash. If none are found
 * this will return NULLPTR.
 */
intermediate_typedef* get_typedef(hash_table* _typedefs, u32 hash)
{
    #if DEBUG
    if (_typedefs->contents == NULLPTR)
        send_error("Intermediate typedefs have not been initted yet");
    #endif

    hash_table_bucket*_bucket = hash_table_at_hash(_typedefs, hash);
    return _bucket == NULLPTR ? NULLPTR : _bucket -> value;
}

/* This creates a typedef and returns a pointer to it. */
intermediate_typedef* add_typedef(intermediate_pass* _pass, \
char* typedef_name, type typedef_type)
{
    hash_table* _typedefs = &_pass->typedefs;

    #if DEBUG
    if (_typedefs->contents == NULLPTR)
        send_error("Intermediate typedefs have not been initted yet");
    #endif

    /* Hashing the typedef name. */
    HASH_STRING(typedef_name);

    /* Checking if the typedef name is valid. */
    if (is_invalid_name(_pass, typedef_name)) {
        printf("\x1b[091mERROR:\x1b[0m The typedef name: \"%s\" is invalid.\n",\
            typedef_name);
        exit(-1);
    }

    /* Check if the typedef name has already been used. */
    if (get_typedef(_typedefs, result_hash) != NULLPTR) {
        printf("\x1b[091mERROR:\x1b[0m The typedef name: \"%s\" has already been used.\n", typedef_name);
        exit(-1);
    }

    // TODO: This is doing the logic twice, once in "get_typedef" and once here.
    hash_table_bucket* _bucket = hash_table_insert_hash(_typedefs, result_hash);

    /* Putting the typedef into a bucket. */
    _bucket->value = malloc(sizeof(intermediate_typedef));
    intermediate_typedef* _typedef = _bucket->value;
    CHECK_MALLOC(_typedef);
    _typedef->hash = result_hash;
    _typedef->name = typedef_name;
    _typedef->type = typedef_type;
    return _typedef;
}

/* This clears all of the intermediate typedefs. */
void clear_intermediate_typedefs(hash_table* _typedefs)
{
    // TODO: There should be a common function to do this that does some magic
    // with function ptrs theres should also be one to go through stacks and
    // vectors like in the "free_intermediate_structs" function.
    hash_table_bucket* current_bucket = _typedefs->contents;
    hash_table_bucket* linked_bucket = NULLPTR;
    for (u32 i=0; i < (1 << _typedefs->size); i++) {
        if (current_bucket->next != NULLPTR) {
            linked_bucket = current_bucket->next;
            do {
                void* tmp = linked_bucket->next;
                if (linked_bucket->value != NULLPTR) {
                    free(linked_bucket->value);
                    linked_bucket->value = NULLPTR;
                }
                free(linked_bucket);
                linked_bucket = tmp;
            } while (linked_bucket != NULLPTR);
        }

        if (current_bucket != NULLPTR && current_bucket->value != NULLPTR)
            free(current_bucket->value);
        current_bucket->value = NULLPTR;
        current_bucket->next = NULLPTR;
        current_bucket->hash = 0;
        current_bucket++;
    }
}

/* This frees all of the intermediate typedefs. */
void free_intermediate_typedefs(hash_table* _typedefs)
{
    clear_intermediate_typedefs(_typedefs);
    free(_typedefs->contents);
}