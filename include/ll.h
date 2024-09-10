#ifndef __LL_H__
#define __LL_H__

#include <stddef.h>

/* Linked List Structure */
struct ll_node {
    struct ll_node *head;	/* First node and handle for a list */
    struct ll_node *prev;
    struct ll_node *next;
    void *obj;			/* Object holding this node */
};

inline void ll_node_remove(struct ll_node *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->head = node;
    node->prev = node;
    node->next = node;
}

void ll_node_clear(struct ll_node *list, void (*destroy)(void *obj));

inline void ll_node_insert(struct ll_node *prev,
			   struct ll_node *node,
			   struct ll_node *next)
{
    node->next = next;
    node->prev = prev;
    prev->next = node;
    next->prev = node;
    node->head = prev->head;
}

void ll_node_add_tail(struct ll_node *list,
		      struct ll_node *node);

inline void ll_node_init(struct ll_node *node, void *obj)
{
    node->head = node;
    node->prev = node;
    node->next = node;
    node->obj = obj;
}

#define LL_HEAD_NODE_INIT(headp) ll_node_init((headp), NULL)

#define LL_NODE_FOREACH(listp, currp, nextp)		\
    for ((currp) = (listp)->head->next,			\
	     (nextp) = (currp)->next;			\
	 (currp) != (listp)->head;			\
	 (currp) = (nextp), (nextp) = (nextp)->next)

#endif
