/*
 * This file runs and controls all of the unit tests while also providing
 * commonly used unit test functions.
 */

#ifndef DEBUG_TESTS_TESTER_H
#define DEBUG_TESTS_TESTER_H

#include<common.h>

/*
 * This is used to tell what percent of tests were passed. This number should
 * always be between 0 and 100 inclusively.
 */
typedef f32 percent;

/* This is the total count of single tests completed. */
extern u32 total_single_test_count;

/* This is the total count of single tests completed successfuly. */
extern u32 total_single_successful_test_count;

/* This function runs all of the unit tests and returns the success percent. */
percent tester_run_all_tests();

/*
 * This hashes the inputted u32. This is used to produce sudo random numbers
 * during unit tests. 
 */
u32 hash_u32(u32 _u32);

/* This inits the variables needed from a shared test function. */
#define TEST_INIT() \
    u32 test_count = 0; \
    percent _percent = 0; \
    bool _result;

/* This adds another single test function's result to the test variables. */
#define TEST_DECLARE(_test) \
    _result = _test; \
    test_count++; \
    total_single_test_count++; \
    total_single_successful_test_count += _result; \
    _percent += (percent)(_result) * 100.0f

/* This adds a shared test function's result to the test variables. */
#define TESTER_DECLARE(_test) \
    test_count++; \
    _percent += (_test); \

/* This gets the percent result of the past tests. */
#define TEST_RESULT() (test_count == 0) ? (0) : (_percent / test_count)

#endif