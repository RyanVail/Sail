#include<std/c/stdlib.h>
#include<std/c/syscall.h>
#include<std/c/string.h>
#include<stdio.h>

// TODO: Make malloc thread safe.

// TODO: just use offsetof.
/* This returns a ptr to the end of the inputted memory segment. */
#define SEGMENT_END_PTR(segment_ptr) ((segment_ptr)->data + \
(size_t)((segment_ptr)->size))

/* This turns a memory segment's data ptr to a main memory segment ptr. */
#define SEGMENT_DATA_TO_MAIN_PTR(data_ptr) (void*)((size_t)data_ptr - \
(size_t)((memory_segment*)NULLPTR)->data)

typedef struct page page;
typedef struct memory_segment memory_segment;

/* This just adds the inputted amount to the malloc data ptr. */
#define GROW_MALLOC_DATA_PTR(amount) \
    malloc_data_ptr = (void*)syscall(SYS_BRK, malloc_data_ptr + (amount));

/* This just adds the inputted amount to the malloc data ptr. */
#define SHRINK_MALLOC_DATA_PTR(amount) \
    malloc_data_ptr = (void*)syscall(SYS_BRK, malloc_data_ptr - (amount));

/* struct page - This represents a page of memory
 * @next: The mext page of memory in the linked list
 * @segments: The memory segments this page contains in a linked list set to
 * NULLPTR if there's no segments in this page and will be in order from the
 * lowest ptr logical value to the highest
 * @size: The number of bytes this page contains
 * @data: The data this page contains
 */
typedef struct page {
    page* next;
    memory_segment* segments;
    u32 size;
    u8 data[1];
} page;

/* struct memory_segment - This represent a single segment of heap allocated
 * memory
 * next: The next segment of memory in the linked list
 * @size: The number of bytes in this segment
 * @data: The data this segment contains
 */
typedef struct memory_segment {
    memory_segment* next;
    u32 size;
    u8 data[1];
} memory_segment;

/* Pages are stored in a linked list inside of the pages of allocted memory. */
static page* pages = NULLPTR;

/* Mmapped regions are stored in their own linked list of "memory_segments"s. */
static memory_segment* mmaped_regions = NULLPTR;

/* The current data ptr. */
static void* malloc_data_ptr = NULLPTR;

/* This preforms a program termiantion with the inputted error code. */
void exit(i32 error_code)
{
    syscall(SYS_EXIT, error_code);

    /* No return function infinite loop */
    for (;;);
}

/* This preforms a termination of the program without doing any clean up. */
void abort()
{
    i32 tgid = syscall(SYS_GETTID);
    i32 pid = syscall(SYS_GETPID);
    syscall(SYS_TGKILL, tgid, pid, SIGABRT);

    /* No return function infinite loop */
    for (;;);
}

/* This tries to remove the inputted page from the linked list. */
static inline void __try_to_remove_page__(page* _page, page* last_page)
{
    page* _tmp_page;
    if (_page->next == NULLPTR) {
        /* Getting rid of this page's data. */
        SHRINK_MALLOC_DATA_PTR(_page->size + sizeof(page) + \
        sizeof(memory_segment));

        /* Trying to remove pages before this page. */
        if (_page == pages) {
            pages = NULLPTR;
        } else {
            last_page->next = _page->next;
            // TODO: This should go through the pages backwards.
            // _tmp_page = last_page->next;
            // last_page->next = NULLPTR;
            // while (last_page != NULLPTR && last_page->segments == NULLPTR) {
            //     _tmp_page = last_page->next;
            //     SHRINK_MALLOC_DATA_PTR(last_page->size + sizeof(page)
            //     + sizeof(memory_segment));
            //     last_page = _tmp_page;
            // }
        }
    }
}

/*
 * TODO: This.
 */
void free(void* ptr)
{
    memory_segment* _segment;
    memory_segment* last_segment;
    page* _page = pages;
    page* last_page = NULLPTR;

    // TODO: A dual linked list would make this O(1).
    /* Going through the pages. */
    for (; _page != NULLPTR; last_page = _page, _page = _page->next) {
        /* Making sure this page has segments. */
        _segment = _page->segments;
        if (_segment == NULLPTR)
            continue;

        /* Going through the segments in this page. */
        last_segment = NULLPTR;
        for (; _segment != NULLPTR; last_segment = _segment, _segment \
        = _segment->next) {
            /* If this is the ptr remove it from the linked list. */
            if (_segment->data == ptr) {
                /* Trying to remove this page. */
                if (last_segment == NULLPTR) {
                    if (_segment->next != NULLPTR) {
                       _page->segments = _segment->next;
                    } else {
                        _page->segments = NULLPTR;
                        __try_to_remove_page__(_page, last_page);
                    }
                } else {
                    last_segment->next = _segment->next;
                }
                return;
            }
        }
    }

    /* Going through the mmaps. */
    // TODO: MMAPS
}

// TODO: Implement this in a better way
void* realloc(void* ptr, size_t bytes)
{
    /* Getting the memory segment from the ptr. */
    memory_segment* _segment = SEGMENT_DATA_TO_MAIN_PTR(ptr);

    /* Making a new segment with the data. */
    void* new_ptr = malloc(bytes);

    /* Copying the old data to the new ptr. */
    if (ptr != NULLPTR) {
        memcpy(new_ptr, ptr, bytes < _segment->size ? bytes : _segment->size);

        /* Freeing the old data. */
        free(_segment->data);
    }

    return new_ptr;
}

static int file_handle;

/*
 * This allocated bytes bytes of memory on the heap and returns a ptr to the
 * allocated memory. In cases of failure this will return NULLPTR.
 */
void* malloc(size_t bytes)
{
    /* Initing malloc. */
    if (malloc_data_ptr == NULLPTR)
        malloc_data_ptr = (void*)syscall(SYS_BRK, NULLPTR);

    /* Size of segments and pages use u32s. */
    if (bytes > __UINT32_MAX__)
        return NULLPTR;

    page* last_page = NULLPTR;
    page* _page = pages;
    u32 bytes_needed;
    memory_segment* _segment = NULLPTR;

    /* Mmaping the data or paging it. */
    if (bytes < MMAP_THREASHOLD) {
        /* Trying to find room for the data in the pages. */
        for (; _page != NULLPTR; last_page = _page, _page = _page->next) {
            if (_page->size <= bytes)
                continue;

            /* Finding space between segments with enough size. */
            _segment = _page->segments;
            while (_segment != NULLPTR && _segment->next != NULLPTR) {
                if ((size_t)_segment->next - (size_t)SEGMENT_END_PTR(_segment) \
                >= bytes + sizeof(memory_segment)) {
                    /* Adding this segment to the linked list. */
                    memory_segment* tmp_segment = _segment->next;
                    _segment->next = _segment+(size_t)SEGMENT_END_PTR(_segment);
                    _segment = _segment->next;
                    _segment->next = tmp_segment;
                    _segment->size = bytes;
                    return _segment->data;
                }
                _segment = _segment->next;
            }

            /* Checking between the last segment and the end of the page. */
            if (_segment != NULLPTR && (size_t)_page->data + \
            (size_t)_page->size - (size_t)SEGMENT_END_PTR(_segment) >= bytes + \
            sizeof(memory_segment))
                break;
        }

        /* If no page with space was found. */
        if (_page == NULLPTR) {
            /* Making sure there's space for the page and memory segment. */
            bytes_needed = bytes + sizeof(page) + sizeof(memory_segment);

            /* This ensures the page size is divisible by the page size mod. */
            bytes_needed += ((bytes_needed % PAGE_GROWTH_MOD_BYTE_SIZE) ? \
            PAGE_GROWTH_MOD_BYTE_SIZE : 0);

            /* Setting the new page ptr. */
            _page = malloc_data_ptr;

            /* Growing the data ptr. */
            // TODO: This doesn't check for errors.
            GROW_MALLOC_DATA_PTR(bytes_needed);

            /* Initing the page. */
            _page->next = NULLPTR;
            _page->segments = NULLPTR;
            _page->size = bytes_needed - sizeof(page) - sizeof(memory_segment);

            /* Adding the page to the linked list. */
            if (last_page == NULLPTR)
                pages = _page;
            else
                last_page->next = _page;

            /* Getting a ptr to the new segment and the first in this page. */
            _page->segments = (memory_segment*)_page->data;
            _segment = _page->segments;
        } else {
            /* If there was a segment the new one is at the end of the last. */
            _segment->next = (memory_segment*)SEGMENT_END_PTR(_segment);
            _segment = _segment->next;
        }

        /* Initing the segment. */
        _segment->next = NULLPTR;
        _segment->size = bytes;

        /* Returning the new segment's data. */
        return _segment->data;
    } else {
        abort();
    }

    abort();
    return NULLPTR;
}

#if __SAIL_MALLOC_DANGLING_FUNCTIONS__
/*
 * This function goes through all of the memory allocations in malloc and
 * returns the number of bytes stored in dangling ptrs. This function is not
 * part of the C standard.
 */
size_t __count_dangling_bytes__()
{
    size_t result = 0;

    /* Counting the mmaped regions. */
    for (memory_segment* _segment = mmaped_regions; _segment != NULLPTR; \
    _segment = _segment->next)
        result += _segment->size;

    /* Counting the segments in pages. */
    for (page* _page = pages; _page != NULLPTR; _page = _page->next)
        for (memory_segment* _segment = _page->segments; _segment != NULLPTR; \
        _segment = _segment->next)
            result += _segment->size;

    return result;
}

/*
 * This function goes through all of the memory allocations in malloc and
 * returns the number of dangling ptrs. This function is not part of the C
 * standard.
 */
u32 __count_dangling_ptrs__()
{
    u32 result = 0;

    /* Counting the mmaped regions. */
    memory_segment* _segment = mmaped_regions;
    for (; _segment != NULLPTR; _segment = _segment->next)
        result++;

    /* Counting the segments in pages. */
    for (page* _page = pages; _page != NULLPTR; _page = _page->next)
        for (_segment = _page->segments; _segment != NULLPTR;  _segment \
        = _segment->next)
            result++;

    return result;
}
#endif
