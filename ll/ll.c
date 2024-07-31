#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "ll.h"

void ll_node_remove(struct ll_node *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->head = node;
    node->prev = node;
    node->next = node;
}

void ll_node_clear(struct ll_node *list, void (*destroy)(void *obj))
{
    struct ll_node *curr, *next;

    curr = list->head->next;
    do {
	next = curr->next;
	ll_node_remove(curr);
	destroy(curr->obj);
	curr = next;
    } while (next != list->head);
}

void ll_node_add_tail(struct ll_node *list, struct ll_node *node)
{
    node->prev = list->prev;
    node->next = list->head;
    list->prev->next = node;
    list->prev = node;
    node->head = list->head;
}

int ll_node_init(struct ll_node *node, size_t objoff)
{
    if (node == NULL)
	return -1;

    node->head = node;
    node->prev = node;
    node->next = node;
    node->obj = (char *) node - objoff;
    return 0;
}
