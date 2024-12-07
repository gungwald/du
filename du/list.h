#ifndef LIST_H_0987890
#define LIST_H_0987890

#include <stdbool.h>    /* bool */
#include <stddef.h>     /* size_t */

struct ListNode
{
    struct ListNode *next;
    wchar_t *data;
};

typedef
    struct ListNode /* as */
    List;

extern List *initList();
extern List *appendListItem(List **l, wchar_t *data);
extern List *skipListItem(const List *l);
extern const wchar_t *getListItem(const List *l);
extern size_t getListSize(const List *l);
extern bool isListEmpty(const List *l);
extern wchar_t *removeListItem(List **l);
extern void printStringList(const List* l);

#endif

