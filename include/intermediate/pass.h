/*
 * This file contains untilites to help make passes through the intermediates
 * easier and remove a lot of copy-pasted code. Intermediate passes and how
 * front ends should interact with them is documented in
 * "documentation/intermediate/passes.md"
 */

#ifndef INTERMEDIATE_PASS_H
#define INTERMEDIATE_PASS_H

#include<common.h>
#include<intermediate/types.h>
#include<datastructures/stack.h>
#include<datastructures/hash_table.h>

typedef struct intermediate_pass intermediate_pass;

#ifndef BACKEND_RTL_RTL_H
    typedef struct rtl rtl;
#endif

/* This is the types of intermediate handler functions. */
typedef void (*handler_func)(intermediate_pass*, intermediate);

/* This is the types of intermediate handler function for intermediate ptrs. */
typedef void (*ptr_handler_func)(intermediate_pass*, intermediate*);

/* This is the type of the intermediate type size handler functions. */
typedef u32 (*type_size_handler_func)(intermediate_pass*, type);

/* This is the type of the intermediate type handler functions. */
typedef void (*type_handler_func)(intermediate_pass*, type);

/* This is the type of the intermediate type kind handler functions. */
typedef void (*type_kind_handler_func)(intermediate_pass*, type);

/* This is the type of the intermediate type reader handler function. */
typedef type (*type_reader_handler_func)(intermediate_pass*, u32 index);

/* This is the type of rtl instruction handler functions. */
typedef void (*rtl_instruction_func)(intermediate_pass*, intermediate);

/*
 * This maps the inputted intermediate type to the inputted function pointer
 * through either the "intermediate_handler_functions" array or if it is not
 * normal the "special_intermediate_handler_funcions". When the intermediate
 * type is reached in a pass the function will be called.
 */
#define INTERMEDIATE_PASS_SET_FUNC(_pass, _type, _func_ptr) \
    if (_type > INTERMEDIATE_TYPE_NORMAL_END) { \
        intermediate_pass_set_special_handler_func(_pass, _type, _func_ptr); \
    } else { \
        _pass->handler_funcs[_type] = _func_ptr; \
    }

/*
 * This sets the index of the inputted type into the special function handler
 * vector to the inputted function ptr. This doesn't do any checks to make sure
 * the inputted index of the vector is present.
 */
#define INTERMEDIATE_PASS_SET_SET_SPECIAL_FUNC_AT(_pass, _type, _func_ptr) \
    *(handler_func*)vector_at(&_pass->special_handler_funcs, _type, false) \
    = _func_ptr

// TODO: There should be flag to preform another function on top of the default
// before or after or something like that.
// TODO: Maybe these should have periods so long blocks of text like this one
// can use something other than commas.
/*
 * struct front_end_pass - This is extra data attached to an intermediate pass
 * for front ends
 * @type_size_func: This fucntion is used to determine the size of the inputted
 * special type
 * @type_printer_func: This function is used to print the value of special types
 * @type_kind_printer_func: This function is used to print special type kinds
 * @type_reader_func: This function is used to read types from strings, this
 * is expected to return a type with NO_TYPE on failures and read the ptr count
 * of types, see get_type in "frontend/common/parser.c"
 * @source_vector: This is the source file, this is a vector of tokens
 * @special_chars: This is the array of special chars, null terminating
 * @white_space_chars: This is the array of white space chars, null terminating
 * @type_names: This is the array of type names, this is null terminating with
 * an empty string
 * @invalid_names: This is the array of invalid names, this is null terminating
 * with an empty string
 */
typedef struct front_end_pass {
    type_size_handler_func type_size_func;
    type_handler_func type_printer_func;
    type_handler_func type_kind_printer_func;
    type_reader_handler_func type_reader_func;
    /* Tokenization things */
    vector source_vector;
    char* special_chars;
    char* white_space_chars;
    /* Arrays of strings */
    char** type_names;
    char** invalid_names;
} front_end_pass;

/*
 * struct rtl_pass - This is extra data attached to an intermediate pass for
 * rtl passes
 * @regs: This is the infinite length array of registers
 * @rtls: This is the output array of rtl instructions
 * @rtl_instruction_func: This is the array of the rtl instruction handler
 * functions
 */
typedef struct rtl_pass {
    vector regs;
    vector rtls;
    // TODO: This 24 magic number should be mapped to the RTL_NIL some how
    rtl_instruction_func handler_funcs[24];
} rtl_pass;

/* struct intermediate_pass - This struct represents all of the specifications
 * of an intermediate pass and values needed during an intermediate pass
 * @intermediates: This is the vector of intermediates
 * @operand_stack: This is the operand stack
 * @recreate: This flag controls wether this pass will recreate the intermediate
 * vector or just change already existing intermediates, if this is set to true
 * handler_funcs should use ptr_handler_func instead
 * @handler_funcs: This is an array of the intermediate handler functions, these
 * functions when an intermediate is read the "intermediate_type" is indexed
 * into this array and the returing function ptr is called, in the case the
 * function ptr is a NULLPTR the default handler function will be called.
 * @special_handler_funcs: This is a vector of intermediate handler functions,
 * these work in the same way "handler_funcs" work but with a dynamic array so
 * the indexes between the desired index and the last index might need to be
 * filled with NULLPTR. This will do no logic when reaching a NULLPTR func ptr.
 * @function_symbols: This is a hash table of "function_symbol" structs as
 * defined in "symboltable.h".
 * @variable_symbols: This is a hash table of "variable_symbol" structs as
 * defined in "symboltable.h".
 * @typedefs: This is the typedef hash table see "intermediate/typedef.c".
 * @structs: This is the struct hash table see "intermediate/struct.c".
 * @enums: This is the enum hash table see "intermediate/enum.c".
 */
typedef struct intermediate_pass {
    /* Intermediates */
    vector intermediates;
    stack operand_stack;
    bool recreate;
    /* Handler functions */
    handler_func* handler_funcs;
    vector special_handler_funcs;
    /* Symbol table data */
    hash_table functions;
    hash_table variables;
    /* Typedefs, structs, and enums */
    hash_table typedefs;
    hash_table structs;
    hash_table enums;
    /* Extra data */
    union data {
        front_end_pass* front_end;
        rtl_pass* rtl;
        void* extra;
    } data;
} intermediate_pass;

/* This preforms the inputted intermediate pass. */
void do_intermediate_pass(intermediate_pass* _pass);

/*
 * This adds the inputted intermediate to the inputted intermediate pass and
 * preforms the normal intermediate pass logic based on the inputted
 * intermediate type. This should be used during front end parsing.
 */
void add_intermediate_to_pass(intermediate_pass* _pass, \
intermediate _intermediate);

/* This creates and returns a new intermediate pass. */
static inline intermediate_pass init_intermediate_pass()
{
    intermediate_pass _pass;
    memset(&_pass, 0, sizeof(intermediate_pass));
    _pass.intermediates.type_size = sizeof(intermediate);

    return _pass;
}

/*
 * This sets the inputted intermediate type index in the special intermediate
 * type handler function to the inputted function ptr. This shouldn't be called
 * directly but through the "INTERMEDIATE_PASS_SET_FUNC" macro.
 */
static inline void intermediate_pass_set_special_handler_func( \
intermediate_pass* _pass, intermediate_type _type, handler_func _func)
{
    /* The size of the special vector. */
    u32 special_size = VECTOR_SIZE(_pass->special_handler_funcs);

    /* If the vector already has enough space for this function. */
    if (special_size < _type) {
        INTERMEDIATE_PASS_SET_SET_SPECIAL_FUNC_AT(_pass, _type, _func);
        return;
    }

    /* If this vector doesn't have enough space append the needed NULLPTRs. */
    void* _nullptr = NULLPTR;
    for (u32 i=0; i < _type - INTERMEDIATE_TYPE_NORMAL_END - special_size; i++)
        vector_append(&_pass->special_handler_funcs, &_nullptr);

    vector_append(&_pass->special_handler_funcs, &_func);
}

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function adds the inputted intermediate back into the intermediate
 * vector preforming no logic at all.
 */
void _add_back_intermediate(intermediate_pass* _pass, intermediate \
_intermediate);

/*
 * This function should only be used as a function ptr in an intermediate pass.
 * This function just returns doing no logic. This should be used when the
 * default intermediate function handler's logic should be skipped. This will
 * not add the intermediate back use the func "_add_back_intermediate" for that.
 */
void _skip_intermediate(intermediate_pass* _pass, intermediate _intermediate);

#endif