/*
 * This holds a lot of functions that apply to the intermediate repersentation
 * of structs.
 */
// TODO: Recursive structs are possible.

#include<frontend/common/parser.h>
#include<intermediate/struct.h>

static hash_table intermediate_structs = { sizeof(struct_variable), NULLPTR };

/* This initializes the intermediate structs hashtable with the inputed size. */
void init_struct_hash_table(u8 hash_table_size)
{
    intermediate_structs = hash_table_init(hash_table_size);
}

/* This frees all of the intermediate structs. */
void free_intermediate_structs()
{
    //
}

/* This returns a pointer to the struct with the same hashed name. */
intermediate_struct* find_struct(u32 struct_hash)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been inited yet");
    #endif

    hash_table_bucket* bucket = \
        hash_table_at_hash(&intermediate_structs, struct_hash);

    return bucket == NULLPTR ? NULLPTR : bucket->value;
}

/*
 * This function attemps the create a struct with "struct_name". Returns a
 * pointer to the newely created struct. Sends errors if any are encountered.
 */
intermediate_struct* create_struct(u8 flags, char* struct_name)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been inited yet");
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
    if (find_struct(result_hash) != NULLPTR) {
        printf("\x1b[091mERROR:\x1b[0m The struct name: \"%s\" has already been used.\n", struct_name);
        exit(-1);
    }

    // TODO: This is doing this logic twice, once in "find_struct" and here.
    hash_table_bucket* struct_bucket = \
        hash_table_insert_hash(&intermediate_structs, result_hash);

    /* Allocating the struct's name*/
    char* new_struct_name = malloc(strlen(struct_name)+1);
    if (new_struct_name == NULLPTR)
        handle_error(0);
    strcpy(new_struct_name, struct_name);

    /* Initing the new struct's contents. */
    struct_bucket->value = malloc(sizeof(intermediate_struct));
    intermediate_struct* _struct = struct_bucket->value;
    if (_struct == NULLPTR)
        handle_error(0);
    _struct->flags = flags;
    _struct->contents.top = NULLPTR;
    _struct->contents.type_size = sizeof(struct_variable);
    _struct->hash = result_hash;
    _struct->name = new_struct_name;
    return _struct;
}

/*
 * This finds and returns the "struct_variable" of the inputed variable. Returns
 * NULLPTR if nothing is found.
 */
struct_variable* get_variable_from_struct(u32 struct_hash, char* var_name)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been inited yet");
    #endif

    /* Getting the struct bucket. */
    hash_table_bucket* bucket = \
        hash_table_at_hash(&intermediate_structs, struct_hash);

    #if DEBUG
    if (bucket == NULLPTR)
        send_error("Unknown struct");
    #endif

    /* Getting the struct. */
    intermediate_struct* _struct = ((intermediate_struct*)bucket->value);

    /* Finding and returning the variable. */
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
 * This adds the inputed variable to the inputed struct. Returns true if it was
 * successful otherwise false.
 */
bool add_variable_to_struct(intermediate_struct* _struct, type var_type, \
char* var_name)
{
    // TODO: This should check if the variable already exists. Maybe split
    // "get_variable_from_struct" into two parts, one part that gets the
    // "intermediate_struct" and the other finds the variable.

    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been inited yet");
    #endif

    #if DEBUG
    if (_struct == NULLPTR)
        send_error("Unknown struct");
    #endif

    /* Allocating the variable name. */
    char* new_var_name = malloc(sizeof(var_name)+1);
    if (new_var_name == NULLPTR)
        handle_error(0);
    strcpy(new_var_name, var_name);

    /* Making the variable "struct_variable". */
    HASH_STRING(var_name);
    struct_variable* struct_var = malloc(sizeof(struct_variable));
    if (struct_var == NULLPTR)
        handle_error(0);

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
        send_error("Intermediate structs have not been inited yet");
    #endif

    /* Making a copy of the struct's variables. */
    stack old_stack = _struct->contents;
    _struct->contents.top = NULLPTR;

    /* Copying over the variables. */
    struct_variable* _var;
    while (stack_size(&old_stack) != 0) {
        _var = stack_pop(&old_stack);
        stack_push(&_struct->contents, _var);
    }
}

/*
 * This returns a pointer to the "intermediate_stucts" hashtable.
 */
intermediate_struct* get_intermediate_structs()
{
    return &intermediate_structs;
}

/*
 * This generates a place holder struct variables in the heap and returns a
 * pointer to it. Padding "struct_variable"s have a hash equal to 0, random type
 * values, and the "name" pointers value is the number of bytes of padding.
 */
struct_variable* generate_padding_struct_variable(u32 bytes_of_padding)
{
    #if DEBUG
    if (intermediate_structs.contents == NULLPTR)
        send_error("Intermediate structs have not been inited yet");
    #endif

    /* Creating the struct variable. */
    struct_variable* struct_var = malloc(sizeof(struct_variable));
    if (struct_var == NULLPTR)
        handle_error(0);

    /* Setting to needed values and returning the struct variable. */
    struct_var->hash = 0;
    struct_var->name = (void*)bytes_of_padding;
    return struct_var;
}