#include <stdio.h>  /* fflush */
#include <wchar.h>	/* wcsdup */
#include <gc.h>
#include "list.h"

List* initList() {
    return NULL;
}

bool isListEmpty(const List *list) {
    return list == NULL;
}

wchar_t* removeListItem(List **l) {
    wchar_t *data;

    data = (*l)->data;
    *l = (*l)->next;
    return data;
}

/* Make sure the data is dynamically allocated and no other code has access to it. */
List* appendListItem(List **l, wchar_t *data) {
    struct ListNode *node;
    struct ListNode *toPutAtEnd;

    toPutAtEnd = (struct ListNode*) GC_MALLOC(sizeof(struct ListNode));
    toPutAtEnd->data = data;
    toPutAtEnd->next = NULL;

    if (*l == NULL) {
        *l = toPutAtEnd;
    } else {
        node = *l;
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = toPutAtEnd;
    }
    return *l;
}

const wchar_t* getListItem(const List *l) {
    return l->data;
}

List* skipListItem(const List *l) {
    return l->next;
}

size_t getListSize(const List *l) {
    const struct ListNode *node;
    size_t i;

    node = l;
    i = 0;
    while (node != NULL) {
        node = node->next;
        i++;
    }
    return i;
}

/* l better be a list of wchar_t * or this will crash. */
void printStringList(const List* l)
{
    const struct ListNode* node;
    node = l;
    while (node != NULL) {
        wprintf(L"%ls\n", node->data);
        node = node->next;
    }
    fflush(stdout);
}

