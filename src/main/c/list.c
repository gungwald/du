#include <malloc.h>
#include "list.h"

List *initList()
{
    return NULL;
}

void freeList(List *l)
{
    void *data;

    while (l != NULL) {
        data = removeListNode(&l);
        free(data);
    }
}

bool isListEmpty(List *list)
{
    return list == NULL;
}

void *removeListNode(List **l)
{
    struct ListNode *detachedHead;
    void *data;

    detachedHead = *l;
    data = (*l)->data;
    *l = (*l)->next;
    free(detachedHead);
    return data;
}

List *appendListNode(List *l, const void *data)
{
    struct ListNode *node;
    struct ListNode *toPutAtEnd;

    node = l;
    while (node != NULL) {
    	node = node->next;
    }
    toPutAtEnd = (struct ListNode *) malloc(sizeof(struct ListNode));
    node->next = toPutAtEnd;
    toPutAtEnd->data = wcsdup(data);
    toPutAtEnd->next = NULL;
    return l;
}

const void *getListNodeData(const List *l)
{
    return l->data;
}

List *skipListNode(const List *l)
{
	return l->next;
}

size_t getListSize(List *l)
{
    struct ListNode *node;
    size_t i;

    node = l;
    i = 0;
    while (node != NULL) {
    	node = node->next;
    	i++;
    }
    return i;
}
