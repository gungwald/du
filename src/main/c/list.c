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
        data = removeListNode(l);
        free(data);
    }
}

bool isListEmpty(List *list)
{
    return list == NULL;
}

/* STOPPED HERE */

void *removeListNode(List *l)
{
	struct ListNode *element;

	element = list->current;

	if (element != NULL) {
		struct ListNode *prev;
		struct ListNode *next;
		
		prev = element->prev;
		next = element->next;

		if (prev != NULL) {
			prev->next = next;
		}
		if (next != NULL) {
			next->prev = prev;
		}
		free(element->data);
		free(element);
		list->current = next;
		if (prev == NULL) {
			list->first = next;
		}
		if (next == NULL) {
			list->last = prev;
		}
		list->size -= 1;
	}
}

List *list_append(List *list, void *data)
{
	struct ListNode *node;

	node = (struct ListNode *) malloc(sizeof(struct ListNode));
	node->data = data;
	node->next = NULL;
	if (list->last == NULL) {
		list->first = node;
		list->last = node;
		list->current = node;
		node->prev = NULL;
	}
	else {
		list->last->next = node;
		node->prev = list->last;
		list->last = node;
	}
	list->size += 1;
	return list;
}

void *list_getData(List *list)
{
	return list->current->data;
}

void list_advance(List *list)
{
	if (list->current != NULL) {
		list->current = list->current->next;
	}
}

void list_reset(List *list)
{
	list->current = list->first;
}

size_t list_getSize(List *list)
{
	return list->size;
}

bool list_hasMoreElements(List *list)
{
	return list->current != NULL;
}
