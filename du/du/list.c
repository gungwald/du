#include <malloc.h>
#include "list.h"

List *list_init()
{
	struct ListControlBlock *list;

	list = (struct ListControlBlock *) malloc(sizeof(struct ListControlBlock));
	list->first = NULL;
	list->current = NULL;
	list->last = NULL;
	list->size = 0;
	return list;
}

void list_free(List *list)
{
	list_reset(list);
	while (! list_isEmpty(list)) {
		list_deleteElement(list);
	}
	free(list);
}

bool list_isEmpty(List *list)
{
	return list->first == NULL;
}

void list_deleteElement(List *list)
{
	struct ListNode *element;

	if (list->current != NULL) {
		element = list->current;
		element->prev->next = element->next;
		element->next->prev = element->prev;
		free(element->data);
		free(element);
		list->size -= 1;
	}
}

List *list_append(List *list, TCHAR *data)
{
	struct ListNode *node;

	node = (struct ListNode *) malloc(sizeof(struct ListNode));
	node->data = _tcsdup(data);
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

TCHAR *list_getData(List *list)
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
