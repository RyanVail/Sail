#include<common.h>
#include<datastructures/stack.h>

/*
 * This adds a value onto the stack.
 */
void stack_push(stack* _stack, void* value)
{
    link* _tmp = _stack->top;
    link* _new_top = malloc(sizeof(link));
    if (_new_top == NULLPTR)
        send_error(0);
    _new_top->next = _tmp;
    _new_top->value = value;
    _stack->top = _new_top;
}

/*
 * This returns a pointer to the value on the top of the stack.
 */
void* stack_top(stack* _stack)
{
    #if DEBUG
    if (_stack->top == NULLPTR) {
        printf("Cannot take the top of a stack with a size of zero.\n");
        abort();
    }
    #endif

    return _stack->top->value;
}

/*
 * This takes the top value off the stack.
 */
void* stack_pop(stack* _stack)
{
    #if DEBUG
    if (_stack->top == NULLPTR) {
        printf("Cannot pop a stack with a size of zero.\n");
        abort();
    }
    #endif

    link* _tmp_link = _stack->top;
    void* _tmp_value = _tmp_link->value;
    _stack->top = _tmp_link->next;
    free(_tmp_link);
    return _tmp_value;
}

/*
 * This returns the size of the stack.
 */
u32 stack_size(stack* _stack)
{
    u32 count = 0;
    link* current = _stack->top;
    while (current != NULLPTR) {
        count++;
        current = current->next;
    }
    return count;
}