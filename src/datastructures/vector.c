#include<common.h>
#include<datastructures/vector.h>
  
/*
 * This makes sure that a vector has been initialized.
 */
// TODO: This function shouldn't exist if "DEBUG" isn't set
static inline void vector_test_valid(vector *_vector)
{
    #if DEBUG
    if (_vector->contents == NULLPTR) {
        printf("Vector has no contents.\n");
        abort();
    } else if (_vector->type_size == 0) {
        printf("Vector doesn't have type size yet.\n");
        abort();
    }
    #else
    // TODO: There should be a flag to remove this line of code.
    if (_vector->contents == NULLPTR || _vector->type_size == 0)
        exit(-1);
    #endif
}

/*
 * This attempts shrink the real size of the vector down.
 */
void vector_try_to_shrink(vector *_vector)
{
    vector_test_valid(_vector);
    if (!_vector->size)
        return;
    if (1 << _vector->size >= _vector->apparent_size << 1) {
        _vector->size--;
        _vector->contents = realloc( \
            _vector->contents, (1 << _vector->size) * _vector->type_size);
        if (_vector->contents == NULLPTR)
            handle_error(1);
    }
}

/*
 * This inits a vector with '1 << size' as a real size.
 */
vector vector_init_with(u8 size_of_items, u8 size)
{
    vector _vector = \
        { malloc(size_of_items*(1 << size)), 0, size, size_of_items};
    return _vector;
}

/*
 * This attempts to expand the real size of the vector to fit the apparent size
 * before new values are added.
 */
void vector_try_to_expand(vector *_vector)
{
    if (_vector->contents == NULLPTR) {
        if (_vector->type_size == 0) {
            #if DEBUG
            printf("Vector doesn't have type size yet.\n");
            abort();
            #endif
            exit(-1);
        }
        _vector->size = 1;
        _vector->contents = malloc(_vector->type_size);
        if (_vector->contents == NULLPTR)
            handle_error(1);
    }
    else if (1 << _vector->size < _vector->apparent_size + 1) {
        _vector->size++;
        _vector->contents = realloc( \
            _vector->contents, (1 << _vector->size) * _vector->type_size);
        if (_vector->contents == NULLPTR)
            handle_error(1);
    }
}

/*
 * This returns a pointer to the value at the index in the vector.
 */
void* vector_at(vector *_vector, u32 index, bool real)
{
    #if DEBUG
    if (!real) {
        if (index >= _vector->apparent_size) {
            printf("Vector has a size of: %u.\n", _vector->apparent_size);
            printf("Vector index is: %u.\n", index);
            send_error("Vector index is too large");
        }
    } else {
        if (index >= 1 << _vector->size) {
            printf("Vector has a real size of: %u.\n",_vector->size);
            printf("Vector index is: %u.\n", index);
            send_error("Vector index is too large");
        }
    }
    if (index < 0)
        send_error("Vector index cannot be under 0");
    #else
    if (!real)
        if (index >= _vector->apparent_size || index < 0)
            exit(-1);
    else
        if (index >= 1 << _vector->size || index < 0)
            exit(-1);
    #endif
    #ifdef _WIN32
    return (u64)_vector->contents + _vector->type_size * index;
    #else
    return _vector->contents + _vector->type_size * index;
    #endif
}

/*
 * This adds the value at the pointer to the end of the vector.
 */
void vector_append(vector *_vector, void* _to_append)
{
    _vector->apparent_size++;
    vector_try_to_expand(_vector);
    vector_test_valid(_vector);
    memcpy(vector_at(_vector, _vector->apparent_size-1, true), \
        _to_append, _vector->type_size);
}

/*
 * This removes and returns the last item in the vector.
 */
void* vector_pop(vector *_vector)
{
    if (_vector->apparent_size == 0) {
        #if DEBUG
        printf("Cannot pop a vector with a size of 0.\n");
        abort();
        #endif
        exit(-1);
    }
    vector_test_valid(_vector);

    void* _tmp = malloc(_vector->type_size);
    if (_tmp == NULLPTR)
        send_error(0);

    memcpy(_tmp, vector_at(_vector, _vector->apparent_size-1, false), \
        _vector->type_size);

    _vector->apparent_size--;
    vector_try_to_shrink(_vector);
    return _tmp;
}
