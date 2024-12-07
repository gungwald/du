#include <stdio.h>  /* fflush */
#include <stdlib.h> /* EXIT_FAILURE */
#include <wchar.h>	/* wcsdup */
#include <gc.h>
#include "strlist.h"

List* initList() {
    struct ListCtrlNode* n;
    n = (struct ListCtrlNode*)GC_MALLOC(sizeof(struct ListCtrlNode));
    if (n == NULL) {
        _wperror(L"Failed to allocate memory for list");
        exit(EXIT_FAILURE);
    }
    n->first = NULL;
    n->cursor = NULL;
    n->last = NULL;
    n->size = 0;
    return n;
}

bool isListEmpty(const List* l) {
    return l->size == 0;
}

wchar_t* removeListItem(List* l) {
    wchar_t* value = NULL;
    struct ListNode* n;

    if (l->size > 0) {
        n = l->cursor;              /* Reference to cursor node */
        if (n == l->first)
            l->first = n->next;
        else
            n->prev->next = n->next;    /* Prev node points to one after cursor */

        if (n == l->last)
            l->last = n->prev;
        else
            n->next->prev = n->prev;    /* Next node points back to one before cursor */
        l->cursor = n->next;        /* Advance cursor */
        l->size--;                  /* List does not contain n */
        value = n->data;
        GC_FREE(n);
    }
    return value;
}

/* Make sure the data is dynamically allocated and no other code has access to it. */
List* appendListItem(List* l, wchar_t* data) {
    struct ListNode* toPutAtEnd;

    toPutAtEnd = (struct ListNode*)GC_MALLOC(sizeof(struct ListNode));
    if (toPutAtEnd == NULL) {
        _wperror(L"Failed to allocate memory for new list item");
        exit(EXIT_FAILURE);
    }
    toPutAtEnd->data = data;
    toPutAtEnd->next = NULL;

    if (l->size == 0) {
        l->first = toPutAtEnd;
        l->cursor = toPutAtEnd;
    } else {
        l->last->next = toPutAtEnd;
    }
    toPutAtEnd->prev = l->last;
    l->last = toPutAtEnd;
    l->size++;

    return l;
}

const wchar_t* getListItem(const List* l) {
    return l->cursor->data;
}

List* advanceList(List* l) {
    if (l->cursor != l->last)
        l->cursor = l->cursor->next;
    return l;
}

bool isListAtEnd(List* l) {
    return l->cursor == NULL;
}

size_t getListSize(const List* l) {
    return l->size;
}

List* resetList(List* l) {
    l->cursor = l->first;
    return l;
}

/* l better be a list of wchar_t * or this will crash. */
void printStringList(List* l)
{
    struct ListNode* origCursor;
    origCursor = l->cursor;
    resetList(l);
    while (!isListAtEnd(l)) {
        wprintf(L"%ls\n", getListItem(l));
        advanceList(l);
    }
    fflush(stdout);
    l->cursor = origCursor;
}

