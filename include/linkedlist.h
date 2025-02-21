#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct node_t node_t;

typedef struct node_t
{
    void * data;
    node_t * next;
    node_t * prev;
} node_t;

typedef struct linkedlist_t
{
    node_t * head;
    node_t * tail;
    size_t count;
} linkedlist_t;

linkedlist_t * linkedlist_init(void);

void linkedlist_append(linkedlist_t * linkedlist, void * data);

void * linkedlist_pop(linkedlist_t * linkedlist);

void * linkedlist_popleft(linkedlist_t * linkedlist);

bool linkedlist_search(linkedlist_t * linkedlist, void * data);

void linkedlist_free_data(linkedlist_t * linkedlist);

void linkedlist_free(linkedlist_t * linkedlist);

#endif 