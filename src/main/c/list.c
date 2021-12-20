#include <wchar.h>	/* wcsdup */
#include <gc.h>
#include "list.h"

List *initList()
{
    return NULL;
}

bool isListEmpty(const List *list)
{
    return list == NULL;
}

void *removeListItem(List **l)
{
    void *data;

    data = (*l)->data;
    *l = (*l)->next;
    return data;
}

List *appendListItem(List **l, void *data)
{
    struct ListNode *node;
    struct ListNode *toPutAtEnd;

    toPutAtEnd = (struct ListNode *) GC_MALLOC(sizeof(struct ListNode));
    toPutAtEnd->data = data;
    toPutAtEnd->next = NULL;

    if (*l == NULL) {
    	*l = toPutAtEnd;
    } else {
		node = *l;
		while (node->next != NULL) {
			node = node->next;
		}
		node->next = toPutAtEnd;
    }
    return *l;
}

const void *getListItem(const List *l)
{
    return l->data;
}

List *skipListItem(const List *l)
{
	return l->next;
}

size_t getListSize(const List *l)
{
    const struct ListNode *node;
    size_t i;

    node = l;
    i = 0;
    while (node != NULL) {
    	node = node->next;
    	i++;
    }
    return i;
}
