#include<common.h>
#include<datastructures/stack.h>

/*
 * This adds a value onto the stack.
 */
void stack_push(stack* _stack, void* value)
{
    link* _tmp = _stack->top;
    link* _new_top = malloc(sizeof(link));
    CHECK_MALLOC(_new_top);
    _new_top->next = _tmp;
    _new_top->value = value;
    _stack->top = _new_top;
}

/* This adds the inputted value onto the stack and allocates space for it. */
void stack_push_malloc_with_size(stack* _stack, void* value, u32 type_size)
{
    /* Copying the value. */
    void* _value = malloc(type_size);
    CHECK_MALLOC(_value);
    memcpy(_value, value, type_size);

    /* Adding a new link. */
    link* _tmp = _stack->top;
    link* _new_top = malloc(sizeof(link));
    CHECK_MALLOC(_new_top);
    _new_top->next = _tmp;
    _new_top->value = _value;
    _stack->top = _new_top;
}

/* This returns the last link on the stack. */
link* stack_last_link(stack* _stack)
{
    link* _tmp_link = _stack->top;

    while (_tmp_link != NULLPTR) {
        if (_tmp_link->next == NULLPTR)
            break;
        _tmp_link = _tmp_link->next;
    }

    return _tmp_link;
}

/* This returns the value of the last link on the stack. */
void* stack_last(stack* _stack)
{
    link* _link = stack_last_link(_stack);
    return _link == NULLPTR ? NULLPTR : _link->value;
}

/* This adds the inputted value to the end of the stack. */
void stack_push_last(stack* _stack, void* value)
{
    link* _tmp_link = stack_last_link(_stack);

    link* _new_top;
    if (_tmp_link == NULLPTR) {
        _stack->top = malloc(sizeof(link));
        _new_top = _stack->top;
    } else {
        _tmp_link->next = malloc(sizeof(link));
        _new_top = _tmp_link->next;
    }

    CHECK_MALLOC(_new_top);
    _new_top->next = NULLPTR;
    _new_top->value = value;
}

/*
 * This takes the top value off the stack and returns its value.
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