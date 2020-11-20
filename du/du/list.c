#include <malloc.h>
#include "list.h"

List *new_List()
{
	struct ListInfo *list;

	list = (struct ListInfo *) malloc(sizeof(struct ListInfo));
	list->first = NULL;
	list->current = NULL;
	list->last = NULL;
	list->size = 0;
	return list;
}

void delete_List(List *list)
{

}

List *listAdd(List *list, TCHAR *data)
{
	struct ListNode *node;

	node = (struct ListNode *) malloc(sizeof(struct ListNode));
	node->data = _tcsdup(data);
	node->next = NULL;
	if (list->last == NULL) {
		list->first = node;
		list->last = node;
		list->current = node;
	}
	else {
		list->last->next = node;
		list->last = node;
	}
	list->size += 1;
	return list;
}

TCHAR *listGetData(List *list)
{
	return list->current->data;
}

void listAdvance(List *list)
{
	if (list->current != NULL) {
		list->current = list->current->next;
	}
}

void listReset(List *list)
{
	list->current = list->first;
}

size_t listGetSize(List *list)
{
	return list->size;
}

bool listHasMoreElements(List *list)
{
	return list->current != NULL;
}
