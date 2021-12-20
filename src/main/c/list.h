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
extern List *appendListItem(List *l, void *data);
extern List *skipListItem(const List *l);
extern const void *getListItem(const List *l);
extern size_t getListSize(const List *l);
extern bool isListEmpty(const List *l);
extern void *removeListItem(List **l);

#endif

