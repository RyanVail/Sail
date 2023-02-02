#include<main.h>
#include<cli.h>
#include<types.h>
#include<datastructures/stack.h>
#include<datastructures/hashtable.h>
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

int main(i32 argc, char* args[])
{
    /* This is a fail safe. */
    #if DEBUG && PTRS_ARE_64BIT
    if (sizeof(void*) != 8)
        send_error( \
        "Set \"PTRS_ARE_64BIT\" flag in \"main.h\" to false and recompile");
    #endif
 
    // TODO: "hashtable.c" should be "hash_table.c" same with "hashtable.h".
    // TOOD: Symbol table ids cannot be stored inside void pointers on 16 bit
    // machines which may be a problem.

    // process_cli_options(argc, args);

    // vector _tmp = tokenize_file("test.sal");
    // for (u32 i=0; i < _tmp.apparent_size; i++) {
    //     printf("%s\n", *(char**)vector_at(&_tmp, i, false));
    // }
    // free_tokenized_file_vector(&_tmp);

    // init_symbol_table(8, 8);

    // C_file_into_intermediate("../tests/fib.c");

    // print_intermediates();

    // optimization_do_register_pass();

    // optimizaiton_do_use_scope_pass();

    // exit(0);

	/* Initing the many many hash tables. */
    init_enum_hash_table(4);
    init_typedef_hash_table(4);
    init_struct_hash_table(4);
    init_symbol_table(8, 8);

    get_global_cli_options()->time_compilation = true;

    // TODO: This and C should be called "into_intermediates" with an 's'.
    salmon_file_into_intermediate("../tests/loop.sal");
    // free_tokenized_file_vector(&_tmp);

    optimization_do_register_pass();
    optimization_do_use_scope_pass();
    optimization_do_constant_pass();

    ARMv7_generate_structs();

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
    // add_variable_symbol("var2", _a, 0);s
    
    // add_variable_symbol("episode", _a, 0);
    // free(_name);
    // printf("%p\n", get_variable_symbol("a", 0)->name);
    // free_tokenized_file_vector(&_tmp);

    // printf("%p\n", get_variable_symbol("", 0));
    // clear_variables_in_scope();
    free_symbol_table();
    free_intermediates(true, true, true);
}
