#pragma once

#include <stdbool.h>
#include <tchar.h>

#define list_size(list) list_getSize(list)
#define list_length(list) list_getSize(list)
#define list_getLength(list) list_getSize(list)

struct ListControlBlock
{
	struct ListNode *first;
	struct ListNode *last;
	struct ListNode *current;
	size_t size;
};

struct ListNode
{
	void *data;
	struct ListNode *prev;
	struct ListNode *next;
};

typedef
	struct ListControlBlock
	List;

extern List   *list_init();
extern void    list_free(List *list);
extern List   *list_append(List *list, TCHAR *data);
extern void    list_advance(List *list);
extern void    list_reset(List *list);
extern void   *list_getData(List *list);
extern size_t  list_getSize(List *list);
extern bool	   list_hasMoreElements(List *list);
extern bool    list_isEmpty(List *list);
extern void    list_deleteElement(List *list);
