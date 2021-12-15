#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "strings.h"
#include "errors.h"

bool isGlob(const wchar_t *s)
{
    return wcschr(s, L'*') != NULL || wcschr(s, L'?') != NULL;
}

wchar_t *concat(const wchar_t *left, const wchar_t *right)
{
    size_t capacity;
    wchar_t *result;

    capacity = wcslen(left) + wcslen(right) + 1;
    if ((result = (wchar_t*)malloc(capacity * sizeof(wchar_t))) == NULL) {
        writeError2(errno, L"malloc failed for concat", left, right);
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
        writeError3(errno,L"Mem alloc failed concat3",first,second,third);
        exit(EXIT_FAILURE);
    } else {
        wcscpy(result, first);
        wcscat(result, second);
        wcscat(result, third);
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
    reqSize=WideCharToMultiByte(CP_UTF8,0,wstr,-1,NULL,0,NULL,NULL);
    utf8 = (char *) malloc(reqSize);
    if (utf8) {
        /* Includes string terminator because of -1 argument. */
        WideCharToMultiByte(CP_UTF8,0,wstr,-1,utf8,reqSize,NULL,NULL);
    } else {
        wperror(L("Failed alloc memory for UTF-8 string"));
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
			utf8StringArray[i] = convertWideCharStringToUtf8(argv[i]);
#else
			utf8StringArray[i] = strdup(argv[i]);
#endif
	}
	return utf8StringArray;
}

void freeStringArray(int elementCount, char *array[])
{
	int i;

	for (i = 0; i < elementCount; i++) {
		free(array[i]);
	}
	free(array);
}

