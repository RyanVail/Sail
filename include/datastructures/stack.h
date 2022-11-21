#ifndef STACK_H
#define STACK_H

#include<common.h>

#define stack_is_empty(_stack) _stack.top == NULL

typedef struct link link;

/* struct link - This is a singly linked link
 * @next: The link after this one
 * @value: A pointer to the value of this link
 */
typedef struct link {
    link* next;
    void* value;
} link;

/* struct stack - This is a simple stack data type
 * @top: This is the value on the top of the stack
 * @type_size: This is the size of the value in one link on the stack
 */
typedef struct stack {
    link* top;
    u8 type_size;
} stack;

/*
 * This adds a value onto the stack.
 */
u32 stack_push(stack* _stack, void* value);

/*
 * This takes the top value off the stack.
 */
void* stack_pop(stack* _stack);

/*
 * This returns the size of the stack.
 */
u32 stack_size(stack* _stack);

#endif
