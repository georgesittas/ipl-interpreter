// Hash table (separate chaining) implementation of a map

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "map.h"

#define MIN_CAP 64

typedef struct {
	void* key;
	void* value;
} KVPair;

typedef struct node {
	KVPair* pair;
	struct node *next;
} Node;

typedef struct list {
	Node* head;
	int size;
} List;

struct map {
	int size;
	int cap;
	List** buckets;

	int (*cmp_keys)(void*, void*);
	void (*destroy_key)(void*);
	void (*destroy_value)(void*);
	unsigned long (*hash_function)(void*);
};

static KVPair* create_kvpair(void* key, void* value) {
	KVPair* new_kvpair = malloc(sizeof(KVPair));
	assert(new_kvpair != NULL);

	new_kvpair->key = key;
	new_kvpair->value = value;

	return new_kvpair;
}

static Node* create_node(KVPair* kvpair, Node* next) {
	Node* new_node = malloc(sizeof(Node));
	assert(new_node != NULL);

	new_node->pair = kvpair;
	new_node->next = next;

	return new_node;
}

static List* create_list(void) {
	List* new_list = malloc(sizeof(List));
	assert(new_list != NULL);

	new_list->head = NULL;
	new_list->size = 0;

	return new_list;
}

static void destroy_list(List* list) {
	assert(list != NULL);
	Node* temp;
	Node* curr = list->head;

	// Don't destroy the keys/values, only the strutures that contain them
	while (curr != NULL) {
		temp = curr;
		curr = curr->next;
		free(temp->pair);
		free(temp);
	}

	free(list);
}

static void add_to_list(List* list, void* kvpair) {
	assert(list != NULL);
	Node* new_node = create_node(kvpair, list->head);
	list->head = new_node;
	list->size++;
}

static Node* find_node_in_list(Map map, List* list, KVPair* pair) {
	if (list == NULL) return NULL;

	Node* curr = list->head;

	while (curr != NULL) {
		if (map->cmp_keys(curr->pair->key, pair->key) == 0) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

static void rehash(Map map) {
	List** old_buckets = map->buckets;
	int old_cap = map->cap;

	map->cap *= 2;
	map->buckets = calloc(map->cap, sizeof(List)); // NULL-initialization here

	void (*destroy_value)(void*) = map->destroy_value;
	map->destroy_value = NULL;

	for (int i = 0; i < old_cap; i++) {
		Node* curr = old_buckets[i]->head;

		while (curr != NULL) {
			map_put(map, curr->pair->key, curr->pair->value);
			curr = curr->next;
		}

		destroy_list(old_buckets[i]);
	}

	map->destroy_value = destroy_value; // Reset old value destructor
}

// Constructs and returns a new empty map
//
// * If cmp_keys is NULL, default_cmp will be used (compares strings)
// * If hash_function is NULL, default_hash will be used (hashes strings)
Map map_create(int (*cmp_keys)(void*, void*),
	             void (*destroy_key)(void*),
	             void (*destroy_value)(void*),
	             unsigned long (*hash_function)(void*)
	            ) {
	Map map = malloc(sizeof(struct map));
	assert(map != NULL);

	map->size = 0;
	map->cap = MIN_CAP;

	map->buckets = calloc(map->cap, sizeof(List)); // NULL-initialization here
	assert(map->buckets != NULL);

	map->cmp_keys = cmp_keys == NULL ? default_cmp : cmp_keys;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	map->hash_function = hash_function == NULL ? default_hash : hash_function;

	return map;
}

// Returns the number of (key, value) pairs in map
int map_size(Map map) {
	assert(map != NULL);
	return map->size;
}

// Inserts a (key, value) pair in map. If key already exists in map,
// the old value will be replaced with the new one
void map_put(Map map, void* key, void* value) {
	assert(map != NULL);
	int idx = map->hash_function(key) % map->cap;

	if (map->buckets[idx] == NULL) {
		map->buckets[idx] = create_list();
	} else {
		KVPair target_pair = { .key = key };
		Node* target_node = find_node_in_list(map, map->buckets[idx], &target_pair);

		if (target_node != NULL) {
			void* old_value = target_node->pair->value;
			target_node->pair->value = value;

			if (map->destroy_value != NULL) {
				map->destroy_value(old_value);
			}

			return;
		}
	}

	add_to_list(map->buckets[idx], create_kvpair(key, value));
	map->size++;

	if (map->size == map->cap) {
		rehash(map);
	}
}

// Retrieves the value that corresponds to key in map
void* map_get(Map map, void* key) {
	assert(map != NULL);
	int idx = map->hash_function(key) % map->cap;
	KVPair target_pair = { .key = key };
	Node* target_node = find_node_in_list(map, map->buckets[idx], &target_pair);
	return target_node == NULL ? NULL : target_node->pair->value;
}

// Frees all memory allocated for map
void map_destroy(Map map) {
	assert(map != NULL);
	for (int i = 0; i < map->cap; i++) {
		Node* temp;

		if (map->buckets[i] == NULL) {
			continue;
		}

		Node* curr = map->buckets[i]->head;

		while (curr != NULL) {
			temp = curr;
			curr = curr->next;

			if (map->destroy_key != NULL) {
				map->destroy_key(temp->pair->key);
			}

			if (map->destroy_value != NULL) {
				map->destroy_value(temp->pair->value);
			}

			free(temp->pair);
			free(temp);
		}

		free(map->buckets[i]);
	}

	free(map->buckets);
	free(map);
}

unsigned long default_hash(void* key) {
	unsigned long hash = 5381;

	for (char *str = (char*) key; *str != '\0'; str++) {
		hash = ((hash << 5) + hash) + *str; 
	}

	return hash; // djb2 hash value is returned
}

int default_cmp(void* keyA, void* keyB) {
	char* strA = (char*) keyA;
	char* strB = (char*) keyB;

	return strcmp(strA, strB);
}
