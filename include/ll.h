#ifndef __LL_H__
#define __LL_H__

#include <stddef.h>

struct ll_node {
    struct ll_node *next;
};

struct ll_list {
    struct ll_link *head;
    size_t dataoff;

    int (*compare)(void *, void *); /* shall return -1 or 0 or 1 */
    int (*getdata)(void *);
};

struct ll_list *ll_list_init(size_t dataoff, size_t datasize);
void ll_list_destroy(struct ll_list *list);

#endif
