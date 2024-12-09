#include <strsafe.h>    /* StringCchLength */
#include "string.h"
#include "error.h"
#include "heap.h"

/* Turn off security warnings about string functions. The correct amount of space is always allocated. */
#pragma warning(disable : 4996)

bool isGlob(const wchar_t *s)
{
    return wcschr(s, L'*') != NULL || wcschr(s, L'?') != NULL;
}

wchar_t *concat(const wchar_t *left, const wchar_t *right)
{
    size_t requiredSize;
    wchar_t *result;

    requiredSize = wcslen(left) + wcslen(right) + 1;
    result = allocateString(requiredSize);
    wcscpy(result, left);
    wcscat(result, right);
    return result;
}

wchar_t *concat3(const wchar_t *first,
                 const wchar_t *second,
                 const wchar_t *third)
{
    size_t requiredSize;
    wchar_t *result;

    requiredSize = wcslen(first) + wcslen(second) + wcslen(third) + 1;
    result = allocateString(requiredSize);
    wcscpy(result, first);
    wcscat(result, second);
    wcscat(result, third);
    return result;
}

wchar_t *concat4(const wchar_t *first,
                 const wchar_t *second,
                 const wchar_t *third,
                 const wchar_t *fourth)
{
    size_t requiredSize;
    wchar_t *result;

    requiredSize = wcslen(first) + wcslen(second) + wcslen(third) + wcslen(fourth)+ 1;
    result = allocateString(requiredSize);
    wcscpy(result, first);
    wcscat(result, second);
    wcscat(result, third);
    wcscat(result, fourth);
    return result;
}

/**
 * s is modified.
 */
wchar_t *replaceAll(wchar_t *s, wchar_t searchFor, wchar_t replaceWith)
{
    wchar_t *p;

    for (p = s; *p != L'\0'; p++) {
        if (*p == searchFor) {
            *p = replaceWith;
        }
    }
    return s;
}

char *convertToUtf8(const wchar_t *wstr)
{
    size_t requiredSize; /* in bytes */
    char *utf8;

    /* Will include string terminator because of -1 argument. */
    requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    utf8 = allocateUTF8String(requiredSize);
    /* Includes string terminator because of -1 argument. */
    WideCharToMultiByte(CP_UTF8,0,wstr,-1,utf8,requiredSize,NULL,NULL);
    return utf8;
}

char **convertAllToUtf8(int argc, const TCHAR *argv[])
{
    char **utf8StringArray;
    int i;

    utf8StringArray = allocateUTF8PointerArray(argc);
    for (i = 0; i < argc; i++) {
        utf8StringArray[i] = convertToUtf8(argv[i]);
    }
    return utf8StringArray;
}

bool startsWith(const wchar_t *s, const wchar_t *prefix) {
    return wcsncmp(s, prefix, wcslen(prefix)) == 0;
}

bool endsWith(const wchar_t *s, const wchar_t *suffix) {
    size_t compareCount, sLength;
    bool endsWith;

    sLength = wcslen(s);
    compareCount = wcslen(suffix);
    if (compareCount > sLength) {
        endsWith = false;
    } else {
        endsWith = wcscmp(s + sLength - compareCount, suffix) == 0;
    }
    return endsWith;
}

bool endsWithChar(const wchar_t *s, wchar_t c) {
    return s[wcslen(s)-1] == c;
}

bool stringContains(const wchar_t *container, const wchar_t *value) {
    return wcsstr(container, value) != NULL;
}

wchar_t *toLowerCase(const wchar_t *s) {
    wchar_t *lower;
    wchar_t *p;

    lower = createStringCopy(s);
    for (p = lower; *p; p++) {
        *p = tolower(*p);
    }
    return lower;
}

wchar_t *createStringCopy(const wchar_t *s) {
    size_t size;
    wchar_t *copy;

    size = findStringLength(s) + 1;
    copy = allocateString(size);
    wcscpy(copy, s);
    return copy;
}

size_t findStringLength(const wchar_t* s)
{
    size_t length;
    if (FAILED(StringCchLength(s, STRSAFE_MAX_CCH, &length))) {
        fail(L"Failed to find length of string");
    }
    return length;
}
