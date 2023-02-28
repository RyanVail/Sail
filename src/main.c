#include<main.h>
#include<cli.h>
#include<types.h>
#include<datastructures/stack.h>
#include<datastructures/hash_table.h>
#include<frontend/salmon/parser.h>
#include<frontend/common/tokenizer.h>
#include<frontend/salmon/preprocessor.h>
#include<frontend/c/preprocessor.h>
#include<frontend/c/parser.h>
#include<frontend/common/preprocessor.h>
#include<intermediate/struct.h>
#include<intermediate/enum.h>
#include<intermediate/symboltable.h>
#include<intermediate/intermediate.h>
#include<intermediate/optimization/registerpass.h>
#include<intermediate/optimization/usescopepass.h>
#include<intermediate/optimization/constantpass.h>
#include<intermediate/typedef.h>
#include<backend/ARMv7.h>
#if linux && DEBUG
#include<time.h>
#endif
#include<frontend/common/parser.h>

char* salmon_file_extensions[] = {"sal", "sah"};
char* c_file_extensions[] = {"c", "h"};

int main(i32 argc, char* args[])
{
    error_handler = NULLPTR;

    /* This is a fail safe. */
    #if DEBUG && PTRS_ARE_64BIT
    if (sizeof(void*) != 8)
        send_error( \
        "Set \"PTRS_ARE_64BIT\" flag in \"main.h\" to false and recompile");
    #endif

    process_cli_options(argc, args);

    // vector _tmp = tokenize_file("test.sal");
    // for (u32 i=0; i < _tmp.apparent_size; i++) {
    //     printf("%s\n", *(char**)vector_at(&_tmp, i, false));
    // }
    // free_tokenized_file_vector(&_tmp);

    // init_symbol_table(8, 8);

    // vector file = C_preprocess_file(file_name);
    // if (file.contents == NULLPTR) {
        // printf("Failed to preprocess C file: %s", file_name);
        // exit(-1);
    // }

    // C_file_into_intermediates("../tests/fib.c");

    // print_intermediates();

    // optimization_do_register_pass();

    // optimization_do_use_scope_pass();

    // exit(0);

	/* Initting the many many hash tables. */


    init_enum_hash_table(4);
    init_typedef_hash_table(4);
    init_struct_hash_table(4);
    init_symbol_table(8, 8);

    // TODO: This currently doesn't free any of the symbols or files after
    // compiling.
    for (u32 i=0; i < VECTOR_SIZE(global_cli_options.input_files); i++) {
        START_PROFILING("compile file", NULLPTR);
        char** file_name = vector_at(&global_cli_options.input_files, i, false);
        // salmon_file_into_intermediates("../tests/loop.sal");
        salmon_file_into_intermediates(*file_name);

        START_PROFILING("do all optimization passes", "compile file");
        optimization_do_register_pass();
        optimization_do_use_scope_pass();
        optimization_do_constant_pass();
        END_PROFILING("do all optimization passes", true);

        generate_structs(&ARMv7_generate_struct);

        END_PROFILING("compile file", false);
    }
    // free_tokenized_file_vector(&_tmp);
    // exit(0);

    // print_intermediates();

    exit(0);

    bin sadfdf = ARMv7_intermediates_into_binary(get_intermediate_vector());

    for (u32 i=0; i < VECTOR_SIZE(sadfdf.contents); i++)
        printf("%08x\n", *(u32*)vector_at(&(sadfdf.contents), i, 0));

    free(sadfdf.contents.contents);
    free(sadfdf.labels.contents);
    ARMv7_free_all();

    // type _a = { 0, U8_TYPE };
    // type _b = { 0, I8_TYPE };
    // type_can_implicitly_cast_to(_a, _b, true);


    // vector* _inputs = malloc(sizeof(vector));
    // _inputs->apparent_size = 0;
    // _inputs->size = 0;
    // _inputs->contents = 0;
    // _inputs->type_size = sizeof(type);

    // printf("%u\n", is_ascii_number("213"));
    // printf("%u\n", check_if_invalid_name("3le_t"));

    // vector_append(_inputs, &_a);

    // type _a = { 0, U8_TYPE };

    // printf("%s\n", get_function_symbol("test", 0)->name);

    // add_function_symbol("foo", _inputs, _a, 0);
    // char* name = "c";
    // char* _name = malloc(2);
    // strcpy(_name, name);
    // printf("----%s\n", _name);
    // add_variable_symbol(_name, _a, 0);

    // add_variable_symbol("var", _a, 0);
    // add_variable_symbol("var2", _a, 0);
    
    // add_variable_symbol("episode", _a, 0);
    // free(_name);
    // printf("%p\n", get_variable_symbol("a", 0)->name);
    // free_tokenized_file_vector(&_tmp);

    // printf("%p\n", get_variable_symbol("", 0));
    // clear_variables_in_scope();
    free_symbol_table();
    free_intermediates(true, true, true);
}

#if DEBUG
#include"debug/debug.c"
#endif