#ifndef LIST_H_0987890
#define LIST_H_0987890

#include <stdbool.h>
#include <tchar.h>

typedef
    struct ListNode
    {
        struct ListNode *next;
        void *data;
    } 
    List;

extern List *initList();
extern List *appendListNode(List *l, const void *data);
extern List *prependListNode(List *l, const void *data);
extern List *skipListNode(const List *l);
extern const void *getListNodeData(const List *l);
extern size_t getListSize(List *l);
extern bool isListEmpty(List *l);
extern void *removeListNode(List **l);
extern void freeList(List *l);

#endif

