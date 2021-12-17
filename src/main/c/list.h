#ifndef LIST_H_0987890
#define LIST_H_0987890

#include <stdbool.h>	/* bool */
#include <stddef.h>		/* size_t */

typedef
    struct ListNode
    {
        struct ListNode *next;
        void *data;
    } 
    List;

extern List *initList();
extern List *appendListItem(List *l, const void *data);
extern List *skipListItem(const List *l);
extern const void *getListItem(const List *l);
extern size_t getListSize(List *l);
extern bool isListEmpty(List *l);
extern void *removeListItem(List **l);
extern void freeList(List *l);

#endif

