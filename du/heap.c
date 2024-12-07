#include <windows.h>
#include "heap.h"

wchar_t* allocateString(size_t sizeInCharsIncludingNull)
{
    /* This will generate an exception if there is no memory available. */
    return (wchar_t *) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeInCharsIncludingNull * sizeof(wchar_t));
}

wchar_t* allocateStringBytes(size_t sizeInBytes)
{
    /* This will generate an exception if there is no memory available. */
    return (wchar_t *) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeInBytes);
}

char* allocateUTF8String(size_t sizeInCharsIncludingNull)
{
    /* This will generate an exception if there is no memory available. */
    return (char *) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeInCharsIncludingNull * sizeof(char));
}

char** allocateUTF8PointerArray(size_t pointerCount)
{
    return (char**)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pointerCount * sizeof(char *));
}

void freeString(wchar_t* s)
{
    if (HeapFree(GetProcessHeap(), 0, s) == FALSE) {
        writeLastError(GetLastError(), L"Failed to free memory for string", s);
    }
}

void freeUTF8String(char* s)
{
    if (HeapFree(GetProcessHeap(), 0, s) == FALSE) {
        writeLastError(GetLastError(), L"Failed to free memory for UTF-8 string");
    }
}

