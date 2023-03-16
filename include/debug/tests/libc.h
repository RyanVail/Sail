/*
 * This file contains unit tests for the self hosted C standard library that
 * should also work with any external C libraries. These functions will not free
 * heap allocated memory if a test fails. These functions also may allocate zero
 * bytes of memory and exit the program because allocation failure which might
 * happen with some implementations of malloc.
 */

#ifndef DEBUG_TESTS_LIBC_H
#define DEBUG_TESTS_LIBC_H

#include<debug/tests/tester.h>

/* This function runs all of the libc tests and returns the success percent. */
percent tester_run_libc_tests();

#if __SAIL_MALLOC_DANGLING_FUNCTIONS__
// TODO: There should be other dangling function tests that don't just free
// memory linearly and that do a lot more tests like with mmaped regions and
// realloc.
/*
 * This tests the Sail libc "__count_dangling_bytes__" function and the
 * "__count_dangling_ptrs__" function by allocating lots of memory in little
 * calls and 1/4 th of the time freeing a random allocation. After it's done
 * allocating the dangling functions are called and tested.
 */
bool tester_run_libc_dangling_test_0();
#endif

/*
 * This tests the libc realloc function by allocating and defining memory then
 * reallocating the memory into a new location in memory. Then testing the old
 * data. The new section of the data then gets defined and tested. This is a
 * single test.
 */
bool tester_run_libc_realloc_test_0();

/*
 * This test insures the libc realloc function will allocate memory in cases
 * that the passed ptr is a NULLPTR.
 */
bool tester_run_libc_realloc_test_1();

/*
 * This test makes sure that reallocing an allocated segment of data to a
 * smaller size copies over the data that will be reallocated.
 */
bool tester_run_libc_realloc_test_2();

/*
 * This tests the libc malloc function by just mass allocating, reallocating,
 * defining, and freeing random numbers of bytes. This will not test the new
 * segment of data after reallocating. This doesn't free the data till the end
 * so devices will a low amount of ram might have troubles with this test. This
 * is a single test.
 */
bool tester_run_libc_alloc_test_0();

#endif