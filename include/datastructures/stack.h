#ifndef STACK_H
#define STACK_H

#include<common.h>

#define STACK_IS_EMPTY(_stack) (_stack.top == NULLPTR)

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
 * This returns a pointer to the value on top of the stack.
 */
void* stack_top(stack* _stack);

/* This returns the value of the last link on the stack. */
void* stack_last(stack* _stack);

/*
 * This takes the top value off the stack.
 */
void* stack_pop(stack* _stack);

/*
 * This returns the size of the stack.
 */
u32 stack_size(stack* _stack);

#endif
