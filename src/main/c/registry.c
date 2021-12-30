#include <stdio.h>
#include <stdlib.h>                 /* EXIT_FAILURE */
#include <stdbool.h>                /* bool, true, false */
#include <lmerr.h>
#include <gc.h>
#include "error.h"
#include "string.h"
#include "filename.h"
#include "registry.h"

#define PATH_ENV_VAR_NAME _T("Path")

int appendToUserPath(_TCHAR *newPathElement)
{
    HKEY environmentKey;
    LSTATUS status;
    TCHAR *path;
    TCHAR *updatedPath;
    _TCHAR *absNewPathElement;

    status = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Environment"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &environmentKey);
    if (status != ERROR_SUCCESS) {
        writeLastError(status, _T("Failed to open registry key"), _T("Environment"));
        exit(EXIT_FAILURE);
    }

    path = getRegistryStringValueForUpdate(environmentKey, PATH_ENV_VAR_NAME);
    absNewPathElement = getAbsolutePath(newPathElement);
    if (stringContains(path, absNewPathElement)) {
        wprintf(L"Directory %ls already exists in user Path in the registry.\n", absNewPathElement);
        wprintf(L"You're ready to run the 'du' command.\n", absNewPathElement);
    } else {
        /* Our installDir is not in the Path yet. */
        _tprintf(_T("Appending %ls to user Path in the registry.\n"), absNewPathElement);
        if (endsWithChar(path, ';')) {
            updatedPath = concat3(path, absNewPathElement, _T(";"));
        } else {
            updatedPath = concat4(path, _T(";"), absNewPathElement, _T(";"));
        }
        setRegistryStringValue(environmentKey, PATH_ENV_VAR_NAME, updatedPath);
    }
    return EXIT_SUCCESS;
}

void setRegistryStringValue(HKEY key, const _TCHAR *valueName, const _TCHAR *data) {
    DWORD sizeInBytes;
    LONG status;
    sizeInBytes = sizeof(_TCHAR) * (_tcslen(data) + 1);
    status = RegSetValueExW(key, valueName, 0, REG_EXPAND_SZ, (const BYTE *) data, sizeInBytes);
    if (status == ERROR_SUCCESS) {
        wprintf(L"Restart your command line window to enable the updated Path.\n");
    } else {
        writeLastError(status, _T("Failed to set registry data for value name"), valueName);
    }
}

TCHAR *getRegistryStringValueForUpdate(HKEY key, const TCHAR *value)
{
    LSTATUS status;
    DWORD dataType;
    DWORD requiredBufferSizeInBytes = 0;
    DWORD retrievedDataSize = 0;
    TCHAR *data = NULL;

    /* Request the size of the buffer required to store the data retrieved from this registry value. */
    status = RegGetValue(key, NULL, value, RRF_RT_ANY, NULL, NULL, &requiredBufferSizeInBytes);
    if (status == ERROR_MORE_DATA || status == ERROR_SUCCESS) {
        /* This is what we wanted to happen. Now we know the required size. */
        data = (TCHAR *) GC_MALLOC(requiredBufferSizeInBytes);
        if (data == NULL) {
            writeError(errno, _T("Failed to allocate memory"), _T("Path environment variable"));
            exit(EXIT_FAILURE);
        }
        retrievedDataSize = requiredBufferSizeInBytes;
        status = RegGetValue(key, NULL, value, RRF_RT_ANY, &dataType, data, &retrievedDataSize);
        if (status != ERROR_SUCCESS) {
            writeLastError(status, _T("Failed to get registry value"), value);
            exit(EXIT_FAILURE);
        }
    } else {
        writeLastError(status, _T("Failed to get required size of buffer for"), _T("Path"));
        exit(EXIT_FAILURE);
    }
    return data;
}
