/*
 * This file is compiled if the "DEBUG" macro is set to true and includes all
 * debug ".c" files for compilation.
 */

#include"profiler.c"

#if UNIT_TESTS
#include"tests/tester.c"
#endif