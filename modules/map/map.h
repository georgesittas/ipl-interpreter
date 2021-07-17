#ifndef MAP_H
#define MAP_H

typedef struct map* Map;

// Constructs and returns a new empty map
//
// * If cmp_keys is NULL, default_cmp will be used (compares strings)
// * If hash_function is NULL, default_hash will be used (hashes strings)
Map map_create(int (*cmp_keys)(void*, void*),
	             void (*destroy_key)(void*),
	             void (*destroy_value)(void*),
	             unsigned long (*hash_function)(void*)
	            );

// Returns the number of (key, value) pairs in map
int map_size(Map map);

// Inserts a (key, value) pair in map. If key already exists in map,
// the old value will be replaced with the new one
void map_put(Map map, void* key, void* value);

// Retrieves the value that corresponds to key in map
void* map_get(Map map, void* key);

// Frees all memory allocated for map
void map_destroy(Map map);

// Compares two strings
int default_cmp(void* strA, void* strB);

// Hashes a string
unsigned long default_hash(void* key);

#endif // MAP_H
