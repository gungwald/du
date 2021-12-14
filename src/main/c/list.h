#pragma once

#include <stdbool.h>
#include <tchar.h>

struct List
{
	void *data;
	struct List *next;
};

extern struct List *addListElemAtTail(List *l, const void *data);
extern struct List *addListElemAtHead(List *l, const void *data);
extern struct List *skipListElem(const List *l);
extern void *getListHeadData(const List *l);
extern size_t getListSize(List *l);
extern bool isListEmpty(List *l);
extern void *removeListHead(List *l);
extern void freeList(List *l);

