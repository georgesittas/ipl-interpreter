#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector* Vector;

// Constructs and returns a new empty vector
Vector vector_create(void (*destroy_item)(void*));

// Returns the size of vector
int vector_size(Vector vector);

// Appends item to the end of vector
void vector_add(Vector vector, void* item);

// Returns pos-th item in vector (starting at 0)
void* vector_get(Vector vector, int pos);

// Frees all memory allocated for vector
void vector_destroy(Vector vector);

#endif // VECTOR_H
