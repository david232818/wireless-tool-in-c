#ifndef __LL_H__
#define __LL_H__

#include <stddef.h>

struct ll_node {
    struct ll_node *head;
    struct ll_node *prev;
    struct ll_node *next;
    void *obj;
};

int ll_node_init(struct ll_node *node, size_t objoff);

#define LL_NODE_INIT(nodep, objtype, nodename) \
    ll_node_init((nodep), offsetof(objtype, nodename))

void ll_node_add_tail(struct ll_node *list, struct ll_node *node);
void ll_node_remove(struct ll_node *node);
void ll_node_clear(struct ll_node *list, void (*destroy)(void *obj));

#define LL_NODE_FOREACH(listp, currp)		\
    for ((currp) = (listp)->head->next;		\
	 (currp) != (currp)->head;		\
	 (currp) = (currp)->next)

#endif
