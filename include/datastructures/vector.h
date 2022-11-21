#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_SIZE(_vector) _vector.apparent_size / sizeof(_vector.type_size)

/* struct vector - This is a dynamicly allocated array
 * @contents: This is a pointer to the start of the string
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
 * This makes sure that a vector has been initialized.
 */
void vector_test_valid(vector *_vector);

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