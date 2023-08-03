#include<common.h>
#include<datastructures/vector.h>
  
/*
 * This attempts shrink the real size of the vector down.
 */
void vector_try_to_shrink(vector *_vector)
{
    #if VECTOR_CHECK_BOUNDS
    if (_vector->contents == NULLPTR)
        send_error("Vector has no contents");
    if (_vector->type_size == 0)
        send_error("Vector doesn't have type size yet");
    #endif

    if (!_vector->size)
        return;
    if (1 << _vector->size >= _vector->apparent_size << 1) {
        _vector->size--;
        _vector->contents = realloc( \
            _vector->contents, (1 << _vector->size) * _vector->type_size);
        if (_vector->contents == NULLPTR)
            HANDLE_COMMON_ERROR(1);
    }
}

/*
 * This inits a vector with (1 << size) as a real size.
 */
vector vector_init(u8 size_of_items, u8 size)
{
    vector _vector = {
        .contents = malloc(size_of_items*(1 << size)),
        .apparent_size = 0,
        .size = size,
        .type_size = size_of_items
    };

    CHECK_MALLOC(_vector.contents);

    return _vector;
}

/*
 * This attempts to expand the real size of the vector to fit the apparent size
 * before new values are added.
 */
void vector_try_to_expand(vector *_vector)
{
    #if VECTOR_CHECK_BOUNDS
    if (_vector->type_size == 0)
        send_error("Vector doesn't have type size yet");
    #endif

    if (1 << _vector->size < _vector->apparent_size + 1) {
        _vector->size++;
        _vector->contents = realloc( \
            _vector->contents, (1 << _vector->size) * _vector->type_size);
        if (_vector->contents == NULLPTR)
            HANDLE_COMMON_ERROR(1);
    }
}

/*
 * This returns a pointer to the value at the index in the vector. This is
 * prefered over directly indexing the content of the vector because it can
 * compile with index checks when the "DEBUG" flag is set to true.
 */
void* vector_at(vector *_vector, u32 index, bool real)
{
    #if VECTOR_CHECK_BOUNDS
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
    #endif
    return (u8*)_vector->contents + (size_t)_vector->type_size * (size_t)index;
}

/*
 * This adds the value at the pointer to the end of the vector.
 */
void vector_append(vector *_vector, void* _to_append)
{
    _vector->apparent_size++;
    vector_try_to_expand(_vector);
    memcpy(vector_at(_vector, _vector->apparent_size-1, true), \
        _to_append, _vector->type_size);
}

/*
 * This removes and returns the last item in the vector.
 */
void* vector_pop(vector *_vector)
{
    #if VECTOR_CHECK_BOUNDS
    if (_vector->contents == NULLPTR)
        send_error("Vector has no contents");
    if (_vector->type_size == 0)
        send_error("Vector doesn't have type size yet");
    #endif

    void* _tmp = malloc(_vector->type_size);
    CHECK_MALLOC(_tmp);

    memcpy(_tmp, vector_at(_vector, _vector->apparent_size-1, false), \
        _vector->type_size);

    _vector->apparent_size--;
    vector_try_to_shrink(_vector);
    return _tmp;
}
