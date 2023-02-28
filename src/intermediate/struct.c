/*
 * This holds a lot of functions that apply to the intermediate representation
 * of structs.
 */
// TODO: Recursive structs are possible.

#include<frontend/common/parser.h>
#include<intermediate/struct.h>

static hash_table intermediate_structs = { sizeof(struct_variable), NULLPTR };

/* This inits the intermediate struct's hashtable with the inputted size. */
void init_struct_hash_table(u8 hash_table_size)
{
    intermediate_structs = hash_table_init(hash_table_size);
}

/* This frees all of the intermediate structs. */
void free_intermediate_structs()
{
    // TODO: Implement this
}

/* This returns a pointer to the struct with the same hashed name. */
intermediate_struct* get_struct(u32 struct_hash)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    hash_table_bucket* bucket = \
        hash_table_at_hash(&intermediate_structs, struct_hash);

    return bucket == NULLPTR ? NULLPTR : bucket->value;
}

/*
 * This function attemps the create a struct with "struct_name". Returns a
 * pointer to the newly created struct. Sends errors if any are encountered.
 */
intermediate_struct* create_struct(u8 flags, char* struct_name)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    /* If this name is invalid. */
    if (is_invalid_name(struct_name)) {
        printf("\x1b[091mERROR:\x1b[0m The struct name: \"%s\" is invalid.\n",\
            struct_name);
        exit(-1);
    }

    /* Hashes the struct name. */
    HASH_STRING(struct_name);

    /* Tests if the struct already exists. */
    if (get_struct(result_hash) != NULLPTR) {
        printf("\x1b[091mERROR:\x1b[0m The struct name: \"%s\" has already been used.\n", struct_name);
        exit(-1);
    }

    // TODO: This is doing this logic twice, once in "get_struct" and here.
    hash_table_bucket* struct_bucket = \
        hash_table_insert_hash(&intermediate_structs, result_hash);

    /* Allocating the struct's name*/
    char* new_struct_name = malloc(strlen(struct_name)+1);
    CHECK_MALLOC(new_struct_name);
    strcpy(new_struct_name, struct_name);

    /* Initting the new struct's contents. */
    struct_bucket->value = malloc(sizeof(intermediate_struct));
    intermediate_struct* _struct = struct_bucket->value;
    CHECK_MALLOC(_struct);
    _struct->flags = flags;
    _struct->contents.top = NULLPTR;
    _struct->hash = result_hash;
    _struct->name = new_struct_name;
    _struct->byte_size = __UINT16_MAX__;
    return _struct;
}

/*
 * This finds and returns the "struct_variable" variable from the inputted
 * struct ptr. This returns NULLPTR if the variable wasn't found.
 */
struct_variable* get_variable_from_struct_ptr(intermediate_struct* _struct, \
char* var_name)
{
    HASH_STRING(var_name);
    link* _var = _struct->contents.top;
    while (true) {
        if (_var == NULLPTR)
            return NULLPTR;
        if (((struct_variable*)_var->value)->hash == result_hash)
            return (struct_variable*)_var->value;
        _var = _var->next;
    }
}

/*
 * This finds and returns the "struct_variable" variable from the inputted
 * struct hash. This returns NULLPTR if the variable wasn't found.
 */
struct_variable* get_variable_from_struct_hash(u32 struct_hash, char* var_name)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    /* Getting the struct bucket. */
    hash_table_bucket* bucket = \
        hash_table_at_hash(&intermediate_structs, struct_hash);

    #if DEBUG
    if (bucket == NULLPTR)
        send_error("Unknown struct variable");
    #endif

    /* Getting the struct. */
    intermediate_struct* _struct = ((intermediate_struct*)bucket->value);

    /* Finding and returning the variable. */
    return get_variable_from_struct_ptr(_struct, var_name);
}

/*
 * This adds the inputted variable to the inputted struct. Returns true if it
 * was successful otherwise false.
 */
bool add_variable_to_struct(intermediate_struct* _struct, type var_type, \
char* var_name)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    #if DEBUG
    if (_struct == NULLPTR)
        send_error("Unknown struct");
    #endif

    /* Checking if the variable already exists. */
    if (get_variable_from_struct_ptr(_struct, var_name) != NULLPTR)
        send_error("Repeated variable entry into struct");

    /* Allocating the variable name. */
    char* new_var_name = malloc(sizeof(var_name)+1);
    CHECK_MALLOC(new_var_name);
    strcpy(new_var_name, var_name);

    /* Making the variable "struct_variable". */
    HASH_STRING(var_name);
    struct_variable* struct_var = malloc(sizeof(struct_variable));
    CHECK_MALLOC(struct_var);

    /* Inserting the variable into the hash table. */
    struct_var->hash = result_hash;
    struct_var->type = var_type;
    struct_var->name = new_var_name;
    stack_push(&_struct->contents, struct_var);
}

/*
 * This goes though all the variables in a struct and reverses their order. This
 * is used after the reconstruction of struct's content's stacks because
 * otherwise all variables would be reversed.
 */
void reverse_struct_variables(intermediate_struct* _struct)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    /* Making a copy of the struct's variables. */
    stack old_stack = _struct->contents;
    _struct->contents.top = NULLPTR;

    /* Copying over the variables. */
    struct_variable* _var;
    while (!STACK_IS_EMPTY(old_stack)) {
        _var = stack_pop(&old_stack);
        stack_push(&(_struct->contents), _var);
    }
}

/*
 * This returns a pointer to the "intermediate_structs" hashtable.
 */
hash_table* get_intermediate_structs()
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    return &intermediate_structs;
}

/*
 * This function goes through all of the defined struct in the hash table and
 * generates their memory layout and anything else using the inputted
 * "struct_generator" function.
 */
void generate_structs(void struct_generator(intermediate_struct*))
{
    START_PROFILING("generate all struct's memory layout", "compile file");

    hash_table_bucket* linked_bucket;
    intermediate_struct* current_struct;

    /* Going through all structs in the struct hash table. */
    for (u32 i=0; i < (1 << intermediate_structs.size); i++) {
        linked_bucket=&((hash_table_bucket*)intermediate_structs.contents)[i];

        /* Going through the linked buckets. */
        while (linked_bucket != NULLPTR) {
            /* If this bucket doesn't have anything just continue. */
            if (linked_bucket->hash == 0) {
                linked_bucket = linked_bucket->next;
                continue;
            }

            /* Making the memory layout of the current struct. */
            current_struct = linked_bucket->value;
            if (current_struct != NULLPTR) {
                reverse_struct_variables(current_struct);
                (*struct_generator)(current_struct);
            }

            linked_bucket = linked_bucket->next;
        }
    }

    END_PROFILING("generate all struct's memory layout", true);
}

/*
 * This generates a place holder struct variables in the heap and returns a
 * pointer to it. Padding "struct_variable"s have a hash equal to 0, random type
 * values and the name is a "u32" that holds the number of bytes of padding.
 */
struct_variable* generate_padding_struct_variable(u32 bytes_of_padding)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been initted yet");
    #endif

    /* Creating the struct variable. */
    struct_variable* struct_var = malloc(sizeof(struct_variable));
    CHECK_MALLOC(struct_var);

    /* Setting to needed values and returning the struct variable. */
    struct_var->hash = 0;
    struct_var->name = (void*)bytes_of_padding;
    return struct_var;
}

#if DEBUG

/*
 * This function prints all of the variables in the struct. This function is for
 * debugging and is not normally called.
 */
void print_struct(intermediate_struct* _struct)
{
    link* current_link = _struct->contents.top;
    struct_variable* current_variable;

    printf("STRUCT: %s\n", _struct->name);

    while (current_link != NULLPTR) {
        /* Getting and printing the struct variable. */
        current_variable = (struct_variable*)current_link->value;
        if (current_variable->hash == 0) {
            printf("PADDING %u BYTES\n", (u32)current_variable->name);
        } else {
            printf("STRUCT VAR: %s\n", current_variable->name);
        }

        /* Going to the next link. */
        current_link = current_link->next;
    }
}

#endif