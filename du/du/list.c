#include <malloc.h>
#include "list.h"

List *list_Init()
{
	struct ListInfo *list;

	list = (struct ListInfo *) malloc(sizeof(struct ListInfo));
	list->first = NULL;
	list->current = NULL;
	list->last = NULL;
	list->size = 0;
	return list;
}

void list_Delete(List *list)
{

}

List *list_Append(List *list, TCHAR *data)
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

TCHAR *list_GetData(List *list)
{
	return list->current->data;
}

void list_Advance(List *list)
{
	if (list->current != NULL) {
		list->current = list->current->next;
	}
}

void list_Reset(List *list)
{
	list->current = list->first;
}

size_t list_GetSize(List *list)
{
	return list->size;
}

bool list_HasMoreElements(List *list)
{
	return list->current != NULL;
}
