#ifndef STRLIST_H_0987890
#define STRLIST_H_0987890

#include <stdbool.h>    /* bool */
#include <stddef.h>     /* size_t */

struct ListNode
{
    struct ListNode* prev;
    struct ListNode* next;
    wchar_t* data;
};

/* It is necessary to keep track of the following fields, like "last", for efficency. The "last" field allows the append
   operation to be done without cycling through the whole list. In other words, in constant time, instead of O(n). The 
   same is true for the "size" operation. */
struct ListCtrlNode
{
    struct ListNode* first;
    struct ListNode* cursor;
    struct ListNode* last;
    size_t size;
};

typedef
struct ListCtrlNode /* as */
    List;

extern List* initList();
extern void appendListItem(const List* l, wchar_t* data);
extern List* advanceList(const List* l);
extern const wchar_t* getListItem(const List* l);
extern size_t getListSize(const List* l);
extern bool isListEmpty(const List* l);
extern wchar_t* removeListItem(const List* l);
extern void printList(const List* l);

#endif

