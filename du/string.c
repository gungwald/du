#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>      /* tolower */
#include <gc.h>
#include "string.h"
#include "error.h"

/* Turn off security warnings about string functions. The correct amount of space is always allocated. */
#pragma warning(disable : 4996)

bool isGlob(const wchar_t *s)
{
    return wcschr(s, L'*') != NULL || wcschr(s, L'?') != NULL;
}

wchar_t *concat(const wchar_t *left, const wchar_t *right)
{
    size_t capacity;
    wchar_t *result;

    capacity = wcslen(left) + wcslen(right) + 1;
    if ((result = (wchar_t *) GC_MALLOC(capacity * sizeof(wchar_t))) == NULL) {
        writeError2(errno, L"GC_MALLOC failed for concat", left, right);
        exit(EXIT_FAILURE);
    } else {
        wcscpy(result, left);
        wcscat(result, right);
    }
    return result;
}

wchar_t *concat3(const wchar_t *first,
                 const wchar_t *second,
                 const wchar_t *third)
{
    size_t reqSize;
    wchar_t *result;

    reqSize = wcslen(first) + wcslen(second) + wcslen(third) + 1;
    result = (wchar_t *) malloc(reqSize * sizeof(wchar_t));
    if (result == NULL) {
        writeError3(errno, L"GC_MALLOC failed concat3", first, second, third);
        exit(EXIT_FAILURE);
    } else {
        wcscpy(result, first);
        wcscat(result, second);
        wcscat(result, third);
    }
    return result;
}

wchar_t *concat4(const wchar_t *first,
                 const wchar_t *second,
                 const wchar_t *third,
                 const wchar_t *fourth)
{
    size_t reqSize;
    wchar_t *result;

    reqSize = wcslen(first) + wcslen(second) + wcslen(third) + wcslen(fourth)+ 1;
    result = (wchar_t *) malloc(reqSize * sizeof(wchar_t));
    if (result == NULL) {
        writeError3(errno, L"GC_MALLOC failed concat3", first, second, third);
        exit(EXIT_FAILURE);
    } else {
        wcscpy(result, first);
        wcscat(result, second);
        wcscat(result, third);
        wcscat(result, fourth);
    }
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
    int reqSize; /* in bytes */
    char *utf8;

    /* Will include string terminator because of -1 argument. */
    reqSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    utf8 = (char *) GC_MALLOC(reqSize);
    if (utf8) {
        /* Includes string terminator because of -1 argument. */
        WideCharToMultiByte(CP_UTF8,0,wstr,-1,utf8,reqSize,NULL,NULL);
    } else {
        _wperror(L"Failed alloc memory for UTF-8 string");
        exit(EXIT_FAILURE);
    }
    return utf8;
}

char **convertAllToUtf8(int argc, const TCHAR *argv[])
{
    char **utf8StringArray;
    int i;

    utf8StringArray = (char **) malloc(sizeof(char *) * argc);
    for (i = 0; i < argc; i++) {
#ifdef UNICODE
        utf8StringArray[i] = convertToUtf8(argv[i]);
#else
        utf8StringArray[i] = strdup(argv[i]);
#endif
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

    size = wcslen(s) + 1;
    copy = (wchar_t *) GC_MALLOC(size * sizeof(wchar_t *));
    if (copy) {
        wcscpy(copy, s);
    } else {
        _wperror(L"Failed to allocate memory for string copy");
        exit(EXIT_FAILURE);
    }
    return copy;
}
