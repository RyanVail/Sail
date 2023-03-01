#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_SIZE(_vector) ((_vector).apparent_size / \
sizeof((_vector).type_size))

/*
 * This returns true if the inputted index is outside of the end bounds of the
 * inputted vector.
 */
#define IS_VEC_END(_vec, _index) ((_index) >= (VECTOR_SIZE(_vec)))

/* struct vector - This is a dynamicly allocated array
 * @contents: This is a pointer to the contents of the vector
 * @apparent_size: This is the size of the vector
 * @size: This is the true size of the vector
 * @type_size: This is the size of one type in the vector
 */
typedef struct vector {
    void* contents;
    u32 apparent_size;
    u8 size;
    u8 type_size;
} vector;

/*
 * This attempts shrink the real size of the vector down.
 */
void vector_try_to_shrink(vector *_vector);

/*
 * This attempts to expand the real size of the vector to fit the apparent size
 * before new values are added.
 */
void vector_try_to_expand(vector *_vector);

/*
 * This inits a vector with (1 << size) as a real size.
 */
vector vector_init(u8 size_of_items, u8 size);

/*
 * This returns the value at the index in the vector.
 */
void* vector_at(vector *_vector, u32 index, bool real);

/*
 * This adds a value to the end of the vector.
 */
void vector_append(vector *_vector, void* _to_append);

/*
 * This removes and returns the last item in the vector.
 */
void* vector_pop(vector *_vector);

#endif
