/*
 * This holds a lot of function that apply to the intermediate representation of
 * typedefs.
 */

#include<intermediate/typedef.h>
#include<frontend/common/parser.h>

static hash_table intermediate_typedefs = { sizeof(intermediate_typedefs), 0 };

/*
 * This initializes the intermediate enum's hashtable with the equation 
 * (1 << "hash_table_size") for the true size.
 */
void init_typedef_hash_table(u8 size)
{
    intermediate_typedefs = hash_table_init(size);
}

/*
 * This finds and returns the typedef with the same hash. If none are found
 * this will return NULLPTR.
 */
intermediate_typedef* get_typedef(u32 hash)
{
    #if DEBUG
    if (intermediate_typedefs.contents == NULLPTR)
        send_error("Intermediate typedefs have not been initted yet");
    #endif

    hash_table_bucket*_bucket = hash_table_at_hash(&intermediate_typedefs,hash);
    return _bucket == NULLPTR ? NULLPTR : _bucket -> value;
}

/* This creates a typedef and returns a pointer to it. */
intermediate_typedef* add_typedef(char* typedef_name, type typedef_type)
{
    #if DEBUG
    if (intermediate_typedefs.contents == NULLPTR)
        send_error("Intermediate typedefs have not been initted yet");
    #endif

    /* Hashing the typedef name. */
    HASH_STRING(typedef_name);

    /* Checking if the typedef name is valid. */
    if (is_invalid_name(typedef_name)) {
        printf("\x1b[091mERROR:\x1b[0m The typedef name: \"%s\" is invalid.\n",\
            typedef_name);
        exit(-1);
    }

    /* Check if the typedef name has already been used. */
    if (get_typedef(result_hash) != NULLPTR) {
        printf("\x1b[091mERROR:\x1b[0m The typedef name: \"%s\" has already been used.\n", typedef_name);
        exit(-1);
    }

    // TODO: This is doing the logic twice, once in "get_typedef" and once here.
    hash_table_bucket* _bucket = hash_table_insert_hash(&intermediate_typedefs,\
        result_hash);

    /* Putting the typedef into a bucket. */
    _bucket->value = malloc(sizeof(intermediate_typedef));
    intermediate_typedef* _typedef = _bucket->value;
    CHECK_MALLOC(_typedef);
    _typedef->hash = result_hash;
    _typedef->name = typedef_name;
    _typedef->type = typedef_type;
    return _typedef;
}