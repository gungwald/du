#ifndef HEAP_0202020202
#define HEAP_0202020202

#include "heap.h"

extern wchar_t* allocateString(size_t sizeInCharsIncludingNull);
extern wchar_t* allocateStringBytes(size_t sizeInBytes);
extern char* allocateUTF8String(size_t sizeInCharsIncludingNull);
extern char** allocateUTF8PointerArray(size_t pointerCount);
extern struct ListCtrlNode* allocateListCtrlNode();
extern void freeString(wchar_t* s);
extern struct ListNode* allocateListNode();
extern void freeListCtrlNode(struct ListCtrlNode* n);
extern void freeListNode(struct ListNode* n);

#endif
