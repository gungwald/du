#pragma once

#include <stdbool.h>
#include <tchar.h>

#define listSize(list) listGetSize(list)
#define listLength(list) listGetSize(list)
#define listGetLength(list) listGetSize(list)

struct ListInfo
{
	struct ListNode *first;
	struct ListNode *last;
	struct ListNode *current;
	size_t size;
};

struct ListNode
{
	TCHAR *data;
	struct ListNode *next;
};

typedef
struct ListInfo
	List;

extern List *new_List();
extern void delete_List(List *list);
extern List *listAdd(List *list, TCHAR *data);
extern void listAdvance(List *list);
extern void listReset(List *list);
extern TCHAR *listGetData(List *list);
extern size_t listGetSize(List *list);
extern bool listHasMoreElements(List *list);
