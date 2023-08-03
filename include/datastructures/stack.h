#ifndef DATASTRUCTURES_STACK_H
#define DATASTRUCTURES_STACK_H

#include<common.h>

#define STACK_IS_EMPTY(_stack) ((_stack).top == NULLPTR)

/*
 * This is a wrapper over "stack_push_malloc_with_size" that gets the type size
 * of the derefrenced value and passes it as the "type_size".
 */
#define STACK_PUSH_MALLOC(_stack, value) \
    stack_push_malloc_with_size((_stack), (value), sizeof((*value)))

typedef struct link link;

/* struct link - This is a singly linked link
 * @next: A pointer to the link after this one
 * @value: A pointer to the value of this link
 */
typedef struct link {
    link* next;
    void* value;
} link;

/* struct stack - This is a simple stack data type
 * @top: This is the value on the top of the stack
 */
typedef struct stack {
    link* top;
} stack;

/* This adds a value onto the stack. */
void stack_push(stack* _stack, void* value);

// TODO: More "stack_push"s calls should use this function instead.
/* This adds the inputted value onto the stack and allocates space for it. */
void stack_push_malloc_with_size(stack* _stack, void* value, u32 type_size);

/*
 * This returns a pointer to the value on the top of the stack.
 */
static inline void* stack_top(stack* _stack)
{
    #if DEBUG
    if (_stack->top == NULLPTR) {
        printf("Cannot take the top of a stack with a size of zero.\n");
        abort();
    }
    #endif

    return _stack->top->value;
}

/* This returns the last link on the stack. */
link* stack_last_link(stack* _stack);

/* This returns the value of the last link on the stack. */
void* stack_last(stack* _stack);

/* This adds the inputted value to the end of the stack. */
void stack_push_last(stack* _stack, void* value);

/*
 * This takes the top value off the stack and returns its value.
 */
void* stack_pop(stack* _stack);

/*
 * This returns the size of the stack.
 */
u32 stack_size(stack* _stack);

#endif
