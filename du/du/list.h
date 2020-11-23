#pragma once

#include <stdbool.h>
#include <tchar.h>

#define list_Size(list) list_GetSize(list)
#define list_Length(list) list_GetSize(list)
#define list_GetLength(list) list_GetSize(list)

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

extern List   *list_Init();
extern void    list_Delete(List *list);
extern List   *list_Append(List *list, TCHAR *data);
extern void    list_Advance(List *list);
extern void    list_Reset(List *list);
extern TCHAR  *list_GetData(List *list);
extern size_t  list_GetSize(List *list);
extern bool	   list_HasMoreElements(List *list);
