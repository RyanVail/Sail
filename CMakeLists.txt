cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_FLAGS_RELEASE "-O2")

project(Sail)

include_directories(include)

add_executable(Sail
	src/main.c
	src/errors.c
	src/types.c
	src/cli.c
	src/datastructures/vector.c
	src/datastructures/stack.c
	src/frontend/common/preprocessor.c
	src/frontend/common/tokenizer.c
	src/frontend/common/parser.c
	src/frontend/salmon/preprocessor.c
	src/frontend/salmon/salmon.c
	src/frontend/c/preprocessor.c
	src/backend/intermediate/optimization/firstpass.c
	src/backend/intermediate/intermediate.c
	src/backend/intermediate/symboltable.c
	src/backend/asm/ARMv7.c
)
