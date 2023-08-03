/*
 * This file contains unit tests for the self hosted C standard library that
 * should also work with any external C libraries. These functions will not free
 * heap allocated memory if a test fails. These functions also may allocate zero
 * bytes of memory and exit the program because allocation failure which might
 * happen with some implementations of malloc.
 */

#include<debug/tests/libc.h>
#include<std/c/stdlib.h>
#include<std/c/stdio.h>

/* This function runs all of the libc tests and returns the success percent. */
percent tester_run_libc_tests()
{
    TEST_INIT();

    START_PROFILING("run libc tests", "run tests");
    TEST_DECLARE(tester_run_libc_alloc_test_0());
    TEST_DECLARE(tester_run_libc_realloc_test_0());
    TEST_DECLARE(tester_run_libc_realloc_test_1());
    TEST_DECLARE(tester_run_libc_realloc_test_2());

    #if __SAIL_MALLOC_DANGLING_FUNCTIONS__
    TEST_DECLARE(tester_run_libc_dangling_test_0());
    #endif

    END_PROFILING("run libc tests", true);
    return TEST_RESULT();
}

#if __SAIL_MALLOC_DANGLING_FUNCTIONS__
// TODO: This should also call realloc.
/*
 * This tests the Sail libc "__count_dangling_bytes__" function and the
 * "__count_dangling_ptrs__" function by allocating lots of memory in little
 * calls and 1/4 th of the time freeing a random allocation. After it's done
 * allocating the dangling functions are called and tested. This test resulting
 * in an error can either be a problem with malloc, realloc, free, and one or
 * more of the dangling functions.
 */
bool tester_run_libc_dangling_test_0()
{
    /* This can't be too high or it will go into a loop. */
    #define ALLOCATION_COUNT 256
    #define RANDOM_SEED 0xe031a8ed

    /* Saving the inital data. */
    size_t inital_dangling_bytes = __count_dangling_bytes__();
    u32 inital_dangling_count = __count_dangling_ptrs__();

    /* This is the data attached to a "dangling" memory allocation. */
    typedef struct dangling_allocation {
        size_t size;
        void* value;
    } dangling_allocation;

    /* The data used to track the allocation data. */
    size_t total_allocation_size = 0;
    u32 total_allocation_count = 0;
    u32 data = RANDOM_SEED;
    dangling_allocation allocation_buf[ALLOCATION_COUNT];
    memset(allocation_buf, 0, sizeof(dangling_allocation) * ALLOCATION_COUNT);

    /* Allocating the data. */
    size_t index = data & 0xFF;
    while (total_allocation_count != ALLOCATION_COUNT) {
        /* 1/4 th of the time data is freed. */
        if ((data >> 16) & 0x3 == 0 && total_allocation_size != 0) {
            /* Finding a valid index. */
            while (allocation_buf[index].size == 0 \
            && index <= total_allocation_count) {
                /* Generating a new random number. */
                data = hash_u32(data);

                /* Checking if this index is valid. */
                index = data & 0xFF;
            }

            /* Freeing the valid index and decrementing the allocation size. */
            total_allocation_size -= allocation_buf[index].size;
            total_allocation_count--;
            allocation_buf[index].size = 0;
            allocation_buf[index].value = NULLPTR;
            free(allocation_buf[index].value);
        } else {
            /* Finding a valid index. */
            while (allocation_buf[index].size != 0 \
            && index <= total_allocation_count) {
                /* Generating a new random number. */
                data = hash_u32(data);

                /* Checking if this index is valid. */
                index = data & 0xFF;
            }

            /* Allocating this "dangling" ptr. */
            allocation_buf[index].size = (data >> 8) & 0xFFF;
            allocation_buf[index].value = malloc(allocation_buf[index].size);
            CHECK_MALLOC(allocation_buf[index].value);
            total_allocation_size += allocation_buf[index].size;
            total_allocation_count++;
        }

        /* Generating the next random number. */
        data = hash_u32(data);
    }

    bool success = true;

    /* Checking the new dangling byte count. */
    if (__count_dangling_bytes__() - inital_dangling_bytes != \
    total_allocation_size)
        success = false;

    /* Checking the new dangling count. */
    if (__count_dangling_ptrs__() - inital_dangling_count != \
    total_allocation_count)
        success = false;

    /* Freeing the data. */
    for (size_t i = 0; i < ALLOCATION_COUNT; i++)
        free(allocation_buf[i].value);

    return success;

    #undef ALLOCATION_COUNT
    #undef RANDOM_SEED
}
#endif

/*
 * This tests the libc realloc function by allocating and defining memory then
 * reallocating the memory into a new location in memory. Then testing the old
 * data. The new section of the data then gets defined and tested. This is a
 * single test.
 */
bool tester_run_libc_realloc_test_0()
{
    #define U32_COUNT 1024

    /* Allocating the memory for the values. */
    u32* ptr = malloc(sizeof(u32) * U32_COUNT);
    CHECK_MALLOC(ptr);

    /* The random seed. */
    #define RANDOM_SEED 0xed0747e7;

    /* Putting data into the ptr. */
    u32 data = RANDOM_SEED;
    for (u32 i=0; i < U32_COUNT; i++) {
        /* Adding the data. */
        *ptr = data;

        /* Generating the next sudo random number and incramenting the ptr. */
        data = hash_u32(data);
        ptr++;
    }

    /* Reallocating the data so that it is in a different location. */
    ptr -= U32_COUNT;
    void* _ptr = ptr;
    u32 u32_count = U32_COUNT;
    while (true) {
        ptr = realloc(ptr, sizeof(u32) * u32_count);
        if (ptr == NULLPTR)
            return false;
        if (_ptr != ptr)
            break;
        u32_count *= 2;
    }

    /* Testing the old data. */
    data = RANDOM_SEED;
    for (u32 i=0; i < U32_COUNT; i++) {
        /* Testing the data. */
        if (*ptr != data)
            return false;

        data = hash_u32(data);
        ptr++;
    }

    /* New random seed. */
    #undef RANDOM_SEED
    #define RANDOM_SEED 0xff43c924

    /* Setting the new section of data. */
    data = RANDOM_SEED;
    for (u32 i=U32_COUNT; i < u32_count - 1; i++) {
        /* Adding the data. */
        *ptr = data;

        /* Generating the next sudo random number and incramenting the ptr. */
        data = hash_u32(data);
        ptr++;
    }

    /* Testing the new section of data. */
    data = RANDOM_SEED;
    for (u32 i=U32_COUNT; i < u32_count; i++) {
        /* Testing the data. */
        if (*ptr != data)
            return false;

        /* Generating the next sudo random number and incramenting the ptr. */
        data = hash_u32(data);
        ptr--;
    }

    /* Freeing the data. */
    free(ptr - U32_COUNT);

    return true;

    #undef U32_COUNT
    #undef RANDOM_SEED
}

/*
 * This test insures the libc realloc function will allocate memory in cases
 * that the passed ptr is a NULLPTR.
 */
bool tester_run_libc_realloc_test_1()
{
    #define BYTE_SIZE 256

    /* Attempting the realloc a NULLPTR. */
    u8* _value = realloc(NULLPTR, BYTE_SIZE);
    if (_value == false)
        return false;

    /* Attempting the set the data. */
    memset(_value, 0, BYTE_SIZE);

    /* Freeing the data and returning successfuly. */
    free(_value);
    return true;

    #undef BYTE_SIZE
}

/*
 * This test makes sure that reallocing an allocated segment of data to a
 * smaller size copies over the data that will be reallocated.
 */
bool tester_run_libc_realloc_test_2()
{
    #define RANDOM_SEED 0x7ef6b56f
    #define INITAL_BYTE_SIZE 432
    #define REALLOCATION_BYTE_SIZE 234

    /* Allocating the data. */
    u8* _ptr = malloc(INITAL_BYTE_SIZE);
    CHECK_MALLOC(_ptr);

    /* Setting the data. */
    u32 data = RANDOM_SEED;
    for (u32 i=0; i < INITAL_BYTE_SIZE; i++) {
        *_ptr = data & 0xFF;
        data = hash_u32(data);
        _ptr++;
    }
    _ptr -= INITAL_BYTE_SIZE;

    /* Reallocating the ptr. */
    _ptr = realloc(_ptr, REALLOCATION_BYTE_SIZE);
    CHECK_MALLOC(_ptr);

    /* Making sure the copied data is the same. */
    data = RANDOM_SEED;
    for (u32 i=0; i < REALLOCATION_BYTE_SIZE; i++) {
        if (*_ptr != (data & 0xFF))
            return false;
        data = hash_u32(data);
        _ptr++;
    }

    /* Freeing the data and returning a success. */
    free(_ptr - REALLOCATION_BYTE_SIZE);
    return true;

    #undef INITAL_BYTE_SIZE
    #undef REALLOCATION_BYTE_SIZE
    #undef RANDOM_SEED
}

/*
 * This tests the libc malloc function by just mass allocating, reallocating,
 * defining, and freeing random numbers of bytes. This will not test the new
 * segment of data after reallocating. This doesn't free the data till the end
 * so devices will a low amount of ram might have troubles with this test. This
 * is a single test.
 */
bool tester_run_libc_alloc_test_0()
{
    #define COUNT 1024
    #define RANDOM_SEED 0x60786943

    /* Storing all of the allocated to free at the end. */
    void* datas[COUNT];

    /* Allocating all of the data. */
    u32 _rand = RANDOM_SEED;
    void* data;
    for (u32 i=0; i < COUNT; i++) {
        /* The first 16 bits are used as the random allocation count. */
        data = malloc(_rand & 0xFFFF);
        u32 goal = _rand & 0xFFFF;
        CHECK_MALLOC(data);

        /* Setting the data. */
        _rand = hash_u32(_rand);
        for (u32 x=0; x < goal; x++) {
            *(u8*)data = _rand & 0xFF;

            /* Generating the next random number. */
            _rand = hash_u32(_rand);
        }

        /* Randomly reallocating the data to the first 17 bits 1/3 the time. */
        if ((_rand >> 24) <= 85) {
            data = realloc(data, _rand & 0x1ffff);
            if (data == NULLPTR)
                return false;
        }

        /* Checking the data. */
        datas[i] = data;

        /* Generating the next random number. */
        _rand = hash_u32(_rand);
    }

    /* Freeing the data. */
    for (u32 i=0; i < COUNT; i++)
        free(datas[i]);

    return true;

    #undef COUNT
    #undef RANDOM_SEED
}
