#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "ll.h"

extern inline void ll_node_remove(struct ll_node *node);

void ll_node_clear(struct ll_node *list, void (*destroy)(void *obj))
{
    struct ll_node *curr, *next;

    LL_NODE_FOREACH(list, curr, next) {
	ll_node_remove(curr);
	destroy(curr->obj);
    }
}

extern inline void ll_node_insert(struct ll_node *prev,
				  struct ll_node *node,
				  struct ll_node *next);

void ll_node_add_tail(struct ll_node *list, struct ll_node *node)
{
    ll_node_insert(list->prev, node, list);
}

extern inline void ll_node_init(struct ll_node *node, void *obj);
