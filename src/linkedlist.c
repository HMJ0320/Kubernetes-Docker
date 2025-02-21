#include "../include/linkedlist.h"

static node_t * node_init(void) {
    node_t * node = (node_t *)malloc(sizeof(node_t));
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

linkedlist_t * linkedlist_init(void) {
    linkedlist_t * linkedlist = (linkedlist_t *)malloc(sizeof(linkedlist_t));
    node_t * head = node_init();
    node_t * tail = node_init();
    linkedlist->head = head;
    linkedlist->tail = tail;
    linkedlist->head->next = linkedlist->tail;
    linkedlist->tail->next = linkedlist->head;
    linkedlist->count = 0;
    return linkedlist;
}

void linkedlist_append(linkedlist_t * linkedlist, void * data) {
    node_t * node = node_init();
    node->data = data;
    if (0 == linkedlist->count) {
        linkedlist->head->next = node;
        linkedlist->tail->prev = node;
        node->next = linkedlist->tail;
        node->prev = linkedlist->head;
        linkedlist->count++;
    } else {
        node_t * tail_prev = linkedlist->tail->prev;
        tail_prev->next = node;
        node->prev = tail_prev;
        node->next = linkedlist->tail;
        linkedlist->tail->prev = node;
        linkedlist->count++;
    }
}

void * linkedlist_pop(linkedlist_t * linkedlist) {
    if (0 == linkedlist->count) {
        return NULL;
    } else {
        node_t * node = linkedlist->tail->prev;
        node->prev->next = linkedlist->tail;
        linkedlist->tail->prev = node->prev;
        linkedlist->count--;
        void * data = node->data;
        free(node);
        return data;
    }
}

void * linkedlist_popleft(linkedlist_t * linkedlist) {
    if (0 == linkedlist->count) {
        return NULL;
    } else {
        node_t * node = linkedlist->head->next;
        linkedlist->head->next = node->next;
        node->next->prev = linkedlist->head;
        linkedlist->count--;
        void * data = node->data;
        free(node);
        return data;
    }
}

bool linkedlist_search(linkedlist_t * linkedlist, void * data) {
    node_t * node = linkedlist->head->next;
    for (size_t i = 0; i < linkedlist->count; i++) {
        if (data == node->data) {
            return true;
        }
        node = node->next;
    }
    return false;
}

void linkedlist_free_data(linkedlist_t * linkedlist) {
    node_t * node = linkedlist->head->next;
    for (size_t i = 0; i < linkedlist->count; i++) {
        free(node->data);
        node = node->next;
    }
}

void linkedlist_free(linkedlist_t * linkedlist) {
    node_t * node = linkedlist->head->next;
    if (0 < linkedlist->count) {
        for (size_t i = 0; i < linkedlist->count; i++) {
            node_t * next = node->next;
            free(node);
            node = next;
        }
    }
    free(linkedlist->head);
    free(linkedlist->tail);
    free(linkedlist);
}
