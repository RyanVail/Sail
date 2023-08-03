cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_FLAGS_RELEASE "--sanitize=address")

project(Sail)

include_directories(include)

# add_compile_options(Sail --sanitize=address)

# TODO: The stdc should be compiled as a library and a lot of other things
# should be done but I'm not good enough at CMake to do them.

# add_executable(Sail
# 	src/main.c
# 	src/errors.c
# 	src/types.c
# 	src/cli.c
# 	src/datastructures/vector.c
# 	src/datastructures/stack.c
# 	src/datastructures/hash_table.c
# 	src/frontend/common/preprocessor.c
# 	src/frontend/common/tokenizer.c
# 	src/frontend/common/parser.c
# 	src/frontend/salmon/preprocessor.c
# 	src/evaluate.c
# 	src/frontend/salmon/parser.c
# 	src/frontend/c/preprocessor.c
# 	src/frontend/c/parser.c
# 	src/intermediate/optimization/registerpass.c
# 	src/intermediate/optimization/ignorepass.c
# 	src/intermediate/optimization/usescopepass.c
# 	src/intermediate/optimization/constantpass.c
# 	src/intermediate/optimization/inequality.c
# 	src/intermediate/optimization/inequalitypass.c
# 	src/intermediate/struct.c
# 	src/intermediate/typedef.c
# 	src/intermediate/enum.c
# 	src/intermediate/intermediate.c
# 	src/intermediate/symboltable.c
# 	src/backend/ARMv7.c
# 	src/backend/rtl/rtl.c
# 	src/std/c/string.c
# 	src/std/c/stdio.c
# 	src/std/c/stdlib.c
# )

# target_compile_definitions(Sail PRIVATE UNIT_TESTS=0 DEBUG=0
# __SAIL_MALLOC_DANGLING_FUNCTIONS__=0)

project(Sail-debug)

add_executable(Sail-debug
	src/main.c
	src/errors.c
	src/types.c
	src/cli.c
	src/archs.c
	src/datastructures/vector.c
	src/datastructures/stack.c
	src/datastructures/hash_table.c
	src/frontend/common/preprocessor.c
	src/frontend/common/tokenizer.c
	src/frontend/common/parser.c
	src/frontend/salmon/preprocessor.c
	src/evaluate.c
	src/frontend/salmon/types.c
	src/frontend/salmon/parser.c
	src/frontend/salmon/struct.c
	# src/frontend/c/preprocessor.c
	# src/frontend/c/parser.c
	src/intermediate/pass.c
	# src/intermediate/optimization/registerpass.c
	# src/intermediate/optimization/ignorepass.c
	# src/intermediate/optimization/usescopepass.c
	src/intermediate/optimization/constantpass.c
	src/intermediate/warning/assignment.c
	# src/intermediate/optimization/inequality.c
	# src/intermediate/optimization/inequalitypass.c
	src/intermediate/struct.c
	src/intermediate/typedef.c
	src/intermediate/enum.c
	src/intermediate/intermediate.c
	src/intermediate/variable.c
	src/intermediate/function.c
	# src/backend/ARMv7.c
	src/backend/rtl/rtl.c
	src/std/c/string.c
	src/std/c/stdio.c
	src/std/c/stdlib.c
)

target_compile_definitions(Sail-debug PRIVATE UNIT_TESTS=1 DEBUG=1
__SAIL_MALLOC_DANGLING_FUNCTIONS__=1)