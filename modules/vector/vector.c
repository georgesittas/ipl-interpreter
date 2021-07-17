// Dynamic array implementation of a vector

#include <stdlib.h>
#include <assert.h>

#include "vector.h"

#define MIN_CAP 64

struct vector {
	void** arr;
	int size;
	int cap;

	void (*destroy_item)(void*);
};

// Constructs and returns a new empty vector
Vector vector_create(void (*destroy_item)(void*)) {
	Vector vector = malloc(sizeof(struct vector));
	assert(vector != NULL);

	vector->arr = calloc(MIN_CAP, sizeof(void*));
	assert(vector->arr != NULL);

	vector->size = 0;
	vector->cap = MIN_CAP;

	vector->destroy_item = destroy_item;
	return vector;
}

// Returns the size of vector
int vector_size(Vector vector) {
	assert(vector != NULL);
	return vector->size;
}

// Appends item to the end of vector
void vector_add(Vector vector, void* item) {
	assert(vector != NULL);

	vector->arr[vector->size++] = item;

	// If we reach the current capacity, grow vector by doubling it
	if (vector->size == vector->cap) {
		vector->cap *= 2;
		vector->arr = realloc(vector->arr, vector->cap * sizeof(void*));
		assert(vector->arr != NULL);
	}
}

// Returns pos-th item in vector (starting at 0)
void* vector_get(Vector vector, int pos) {
	assert(vector != NULL && pos >= 0 && pos < vector->size);
	return vector->arr[pos];
}

// Frees all memory allocated for vector
void vector_destroy(Vector vector) {
	assert(vector != NULL);

	for (int i = 0; i < vector->size; i++) {
		if (vector->destroy_item != NULL) {
			vector->destroy_item(vector->arr[i]);
		}
	}

	free(vector->arr);
	free(vector);
}
