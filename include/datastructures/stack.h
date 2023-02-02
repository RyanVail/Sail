#ifndef STACK_H
#define STACK_H

#include<common.h>

#define IS_STACK_EMPTY(_stack) (_stack.top == NULLPTR)

typedef struct link link;

/* struct link - This is a singly linked link
 * @next: A pointer to the link after this one
 * @value: A pointer to the value of this link
 */
typedef struct link {
    link* next;
    void* value;
} link;

// TODO: type_size is not used.
/* struct stack - This is a simple stack data type
 * @top: This is the value on the top of the stack
 * @type_size: This is the size of the values inside the links
 */
typedef struct stack {
    link* top;
    u8 type_size;
} stack;

/*
 * This adds a value onto the stack.
 */
// TODO: The allocation and transfter of data should be done in "stack_push"
// to remove a large chunk of repetative code.
void stack_push(stack* _stack, void* value);

/*
 * This returns a pointer to the value on top of the stack.
 */
void* stack_top(stack* _stack);

/*
 * This takes the top value off the stack.
 */
void* stack_pop(stack* _stack);

/*
 * This returns the size of the stack.
 */
u32 stack_size(stack* _stack);

#endif
