#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linked_list.h"

void linked_list_init(linked_list_t *ll) {
    ll->tail = NULL;
    ll->head = NULL;
    ll->size = 0;
}

void linked_list_push(linked_list_t *ll, int x, char *text) {
    node_t *node = malloc(sizeof(node_t));
    assert(node != NULL);
    node->x = x;
    node->text = text;
    node->next = NULL;
    
    if (ll->tail == NULL) {
        ll->tail = node;
        ll->head = node;
    } else {
        ll->head->next = node;
        ll->head = node;
    }

    ll->size++;
}

node_t *linked_list_pop(linked_list_t *ll) {
    if (ll->tail != NULL) {
        node_t *old_tail = ll->tail;
        node_t *new_tail = old_tail->next;
        ll->tail = new_tail;
        ll->size--;

        return old_tail;
    }
    return NULL;
}

void linked_list_clear_node(node_t *node) {
    node->next = NULL;
    free(node->text);
    free(node);
}

void linked_list_clear(linked_list_t *ll) {
    node_t *node;
    while ((node = linked_list_pop(ll))) linked_list_clear_node(node);
}

/* Inner function */
void linked_list_print_nodes_recursively(node_t *node) {
    if (node == NULL) return;
    printf("%d %s\n", node->x, node->text);
    linked_list_print_nodes_recursively(node->next);
}

void linked_list_print(linked_list_t *ll) {
    linked_list_print_nodes_recursively(ll->tail);
}
