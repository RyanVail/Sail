#include<main.h>
#include<cli.h>
#include<types.h>
#include<datastructures/stack.h>
#include<frontend/salmon/salmon.h>
#include<frontend/common/tokenizer.h>
#include<frontend/salmon/preprocessor.h>
#include<frontend/c/preprocessor.h>
#include<frontend/common/preprocessor.h>
#include<backend/intermediate/symboltable.h>
#include<backend/intermediate/intermediate.h>
#include<backend/intermediate/optimization/firstpass.h>

int main(i32 argc, char* args[])
{
    #if DEBUG
    #if VOID_PTR_64BIT
    /* This is a secondary fail safe. */
    if (sizeof(void*) != 8)
        send_error( \
        "Set \"VOID_PTR_64BIT\" flag in \"main.h\" to false and recompile");
    #else
    if (sizeof(void*) >= 8)
        send_error( \
        "Set \"VOID_PTR_64BIT\" flag in \"main.h\" to true and recompile");
    #endif
    #endif
    // TOOD: Symbol table ids cannot be stored inside void pointers on 16bit
    // machines which may be a problem.

    // process_cli_options(argc, args);

    // vector _tmp = tokenize_file("/home/ryan/Documents/PDFs/Armv8-A Instruction Set Architecture.pdf");
    // vector _tmp = tokenize_file("test.sal");
    // for (u32 i=0; i < _tmp.apparent_size; i++) {
    //     printf("%s\n", *(char**)vector_at(&_tmp, i, false));
    // }
    // free_tokenized_file_vector(&_tmp);

    vector c_file = C_preprocess_file("../tests/fib.c");
    for (u32 i=0; i < VECTOR_SIZE(c_file); i++) {
        printf("%s\n", *(char**)vector_at(&c_file, i, 0));
    }
/*
    salmon_file_into_intermediate("../tests/loop.sal");
    // free_tokenized_file_vector(&_tmp);

    optimization_do_first_pass();
    print_intermediates();

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

    free_symbol_table();
    free_intermediates();*/
}
