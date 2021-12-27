#include <stdio.h>
#include <string.h>
#include <lmerr.h>
#include "du.h"
#include "error.h"

#define ERROR_TEXT_CAPACITY 128

static void displayErrorText(DWORD errorCode);

/* This function was taken from Microsoft's Knowledge Base Article 149409
   and modified to fix the formatting. */
void displayErrorText(DWORD errorCode)
{
    HMODULE moduleHandle = NULL; /* default to system source */
    _TCHAR* message;
    DWORD bufferLength;

    /* If errorCode is in the network range, load the message source */
    if (errorCode >= NERR_BASE && errorCode <= MAX_NERR) {
        moduleHandle = LoadLibraryEx(_T("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (moduleHandle == NULL) {
            /* Can't call writeLastError because that could cause an infinite recursive failure loop. */
            _ftprintf(stderr, _T("failed to load library netmsg.dll: error number %lu\n"), GetLastError());
        }
    }

    /* Call FormatMessage() to allow for message text to be acquired
       from the system or the supplied module handle */
    bufferLength = FormatMessage(
                       FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       FORMAT_MESSAGE_FROM_SYSTEM | /* always consider system table */
                       ((moduleHandle != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
                       moduleHandle, /* Module to get message from (NULL == system) */
                       errorCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
                       (LPTSTR)&message,
                       0,
                       NULL);

    if (bufferLength) {
        /* Output message string on stderr */
        _ftprintf(stderr, message);
        /* WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, bufferLength, &numberOfBytesWritten, NULL); */
        /* Free the buffer allocated by the system */
        LocalFree(message);
    }
    /* If you loaded a message source, unload it */
    if (moduleHandle != NULL) {
        FreeLibrary(moduleHandle);
    }
}

void writeError(errno_t errorCode, const _TCHAR* message, const _TCHAR* object)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

    _tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
    _ftprintf(stderr, _TEXT("%ls: %ls: \"%ls\": %ls\n"), programName, message, object, errorText);
}

void writeError2(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR* object2)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

    _tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
    _ftprintf(stderr, _TEXT("%ls: %ls: \"%ls\" and \"%ls\": %ls\n"), programName, message, object1, object2, errorText);
}

void writeError3(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR* object2, const _TCHAR* object3)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

    _tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
    _ftprintf(stderr, _TEXT("%ls: %ls: \"%ls\", \"%ls\" and \"%ls\": %ls\n"), programName, message, object1, object2, object3, errorText);
}

void writeLastError(DWORD lastError, const _TCHAR* message, const _TCHAR* object)
{
    _ftprintf(stderr, _TEXT("%ls: %ls: %ls: "), programName, message, object);
    displayErrorText(lastError);
    /* _ftprintf(stderr, _TEXT("\n")); */
    fflush(stderr);
}

void writeLastError2(DWORD lastError, const TCHAR* message, const TCHAR* object1, const TCHAR *object2)
{
    _ftprintf(stderr, _TEXT("%ls: %ls: %ls and %ls: "), programName, message, object1, object2);
    displayErrorText(lastError);
}
