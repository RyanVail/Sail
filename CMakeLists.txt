cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_FLAGS_RELEASE "-O2")

project(Sail)

include_directories(include)

add_executable(Sail
	src/main.c
	src/cli.c
	src/errors.c
	src/frontend/common/tokenizer.c
	src/datastructures/vector.c
	src/datastructures/stack.c
	src/frontend/salmon/salmon.c
	src/types.c
	src/backend/intermediate/symboltable.c
	src/frontend/common/preprocessor.c
	src/frontend/salmon/preprocessor.c
	src/common.c
	src/frontend/common/parser.c
)
