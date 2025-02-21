/** @file hashtable.h
 *
 * @brief Hashtable datastructure userd to store key-value pairs.
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The maximum size of the hashtable, prime number due to less collisions.
//
#define TABLE_SIZE 211

// The maximum size of a any string.
//
#define MAX_SIZE   64

// Struct that holds the key value pairs and the next item for collision cases
//
typedef struct item_t
{
    void *          key;
    void *          value;
    struct item_t * next;
} item_t;

// Defines hashtable structure which is an array of nodes.
//
typedef struct hashtable_t
{
    item_t ** table;
} hashtable_t;

// Initializes the hashtable datastructure.
//
hashtable_t * hashtable_init(void); // create a new hash table

// Inserts the key-value pair into the hashtable based on the key type.
//
void hashtable_insert(hashtable_t *hashtable, void *key, void *value, char key_type);

// Searches the hashtable to see if a value exists for the key specified.
//
void *hashtable_search(hashtable_t *hashtable, void *key, char key_type);

// Removes the key-value pair if the key exists in the hashtable.
//
void hashtable_remove(hashtable_t *hashtable, void *key, char key_type);

// Frees any data that was allocated for the hashtable.
//
void hashtable_free(hashtable_t * hash_table);

// Frees any values for key-value pairs in the hashtable.
//
void hashtable_free_values(hashtable_t * hash_table);

#endif /* hashtable.h */

/*** end of file ***/