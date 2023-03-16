/*
 * This file handles internall profiling of code passes. Functions in in this
 * file provide a way to easily start and stop profiles.
 */

#include<cli.h>
#include<debug/profiler.h>
#include<datastructures/stack.h>
#include<datastructures/hash_table.h>
#include<time.h>
#if _WIN32
#include <sys/timeb.h>
#endif

/* struct profile - This represents a debuging profile
 * @starting_time: The starting time of this profile
 * @ending_time: The ending time of this profile
 * @children: The children of this profile
 * @name: The name of this profile
 * @hash: The hash of the name of this profile
 * @depth: The depth of this profile
 * @has_parent: If this profile is a child of another profile or not
 */
typedef struct profile {
    #if linux
    clock_t starting_time;
    clock_t ending_time;
    #elif _WIN32
    time_t starting_time;
    time_t ending_time;
    #endif
    stack children;
    char* name;
    u32 hash;
    u8 depth;
    bool has_parent;
} profile;

static stack profile_stack = { NULLPTR };

/*
 * This searches the inputed stack for the inputed profile name's hash. The
 * "clear" input boolean determines if the link the profile came from will be
 * freeded or not, and should only be set to true if the profile is being ended.
 */
profile* debug_profiler_search(u32 profile_name_hash, stack* _stack, bool clear)
{
    link* last_link = NULLPTR;
    link* current_link = _stack->top;
    for (; current_link != NULLPTR; last_link = current_link, \
    current_link = current_link->next) {
        if (current_link->value == NULLPTR)
            continue;

        if (((profile*)current_link->value)->hash == profile_name_hash) {
            profile* _value = current_link->value;
            if (clear && _value->has_parent) {
                if (last_link != NULLPTR)
                    last_link->next = current_link->next;
                else
                    _stack->top = current_link->next;

                free(current_link);
            }
            return _value;
        }

        /* If this profile has children recursivly search through them. */
        if (((profile*)current_link->value)->children.top != NULLPTR) {
            profile* tmp_profile = debug_profiler_search(profile_name_hash, \
            &((profile*)current_link->value)->children, clear);
            if (tmp_profile != NULLPTR)
                return tmp_profile;
        }
    }
    return NULLPTR;
}

/*
 * This starts a profile for the inputed profile name. If the parent input
 * isn't "NULLPTR" this will put the profile under the parent.
 */
void debug_profiler_start(char* profile_name, char* parent_name)
{
    if (!global_cli_options.time_compilation)
        return;

    u32 parent_name_hash;
    stack* _stack;
    u8 depth;

    /* Finding the stack to add the profile to. */
    if (parent_name != NULLPTR) {
        // TODO: "HASH_STRING" needs to be better so this isn't as disgusting.
        {
        HASH_STRING(parent_name)
        parent_name_hash = result_hash;
        }

        profile* parent = debug_profiler_search(parent_name_hash, \
        &profile_stack, false);

        if (parent == NULLPTR)
            send_error("Adding profile failed, can't find parent");

        _stack = &parent->children;
        depth = parent->depth + 1;
    } else {
        _stack = &profile_stack;
        depth = 0;
    }

    /* Creating the new name. */
    char* new_name = malloc(strlen(profile_name)+1);
    CHECK_MALLOC(new_name);
    new_name[strlen(profile_name)];
    strcpy(new_name, profile_name);

    HASH_STRING(profile_name);

    /* Creating the new profile. */
    profile* new_profile = malloc(sizeof(profile));
    CHECK_MALLOC(new_profile);
    new_profile->children.top = NULLPTR;
    new_profile->has_parent = parent_name != NULLPTR;
    new_profile->depth = depth;
    new_profile->hash = result_hash;
    new_profile->name = new_name;
    new_profile->ending_time = (time_t)0;
    #if _WIN32
    time(&new_profile->starting_time);
    #else
    new_profile->starting_time = clock();
    #endif

    /* Adding the profile to the stack. */
    // stack_push_last(_stack, new_profile); // TODO: Why does using
    // "stack_push_last" break this???
    stack_push(_stack, new_profile);
}

/* "print_profile" will also free the links in the profiles. */
void debug_profiler_end_profile(profile* _profile, bool print_profile)
{
    if (!global_cli_options.time_compilation)
        return;

    if (_profile == NULLPTR)
        send_error("Ending profile failed");

    if (print_profile) {
        /* Printing the spaces. */
        for (u32 i=0; i < _profile->depth; i++)
            printf("    ");

        /* Adding the dot character. */
        char point_to_print;
        switch (3 & _profile->depth)
        {
        case 0:
            point_to_print = '*';
            break;
        case 1:
            point_to_print = '+';
            break;
        case 2:
            point_to_print = '-';
            break;
        case 3:
            point_to_print = '~';
            break;
        }
        printf("%c ", point_to_print);

        #if _WIN32
        printf("Took %f ms to %s\n", (f32)(_profile->ending_time - \
        _profile->starting_time) / 1000.0f);
        #else
        printf("Took %f ms to %s\n", ((f32)_profile->ending_time - \
        _profile->starting_time) / CLOCKS_PER_SEC * 1000.0f, _profile->name);
        #endif
    }

    for (link* current_link = _profile->children.top; current_link != NULLPTR;){
        /*
         * If this profile doesn't have an ending time it inherits this
         * profile's ending time.
         */
        if (((profile*)current_link->value)->ending_time == 0)
            ((profile*)current_link->value)->ending_time =_profile->ending_time;

        debug_profiler_end_profile(current_link->value, print_profile);

        link* next = current_link->next;
        if (print_profile)
            free(current_link);
        current_link = next;
    }

    if (_profile->has_parent)
        return;

    free(_profile->name);
    free(_profile);
}

// TODO: Try and remove "has_parent" inputted boolean because it's dumb.
/*
 * This stops and prints stats on the inputed profile name. This will also
 * recursivly end any children profiles. "has_parent" should be true if this
 * profile has a parent.
 */
void debug_profiler_end(char* profile_name, bool has_parent)
{
    if (!global_cli_options.time_compilation)
        return;

    #if _WIN32
    time_t ending_time;
    time(&ending_time);
    #else
    clock_t ending_time = clock();
    #endif

    HASH_STRING(profile_name);
    profile* _profile = debug_profiler_search(result_hash, &profile_stack, \
    !has_parent);

    if (has_parent && !_profile->has_parent)
        send_error("\"has_parent\" should be set to true for a child profile");

    if (_profile->ending_time == 0)
        _profile->ending_time = ending_time;

    debug_profiler_end_profile(_profile, !_profile->has_parent);
}

/* This stops all current profiles. */
void debug_profiler_end_all()
{
    if (!global_cli_options.time_compilation)
        return;
    // TODO: Implement this.
}