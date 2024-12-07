#include <stdio.h>
#include <stdlib.h>                 /* EXIT_FAILURE */
#include <stdbool.h>                /* bool, true, false */
#include <lmerr.h>
#include <gc.h>
#include "error.h"
#include "string.h"
#include "filename.h"
#include "registry.h"

#define REGISTRY_ENVIRONMENT_KEY_NAME _T("Environment")
#define REGISTRY_USER_PATH_VALUE_NAME _T("Path")

static const _TCHAR *addElementToPath(const _TCHAR *path, const _TCHAR *element);

void addElementToRegistryUserPath(const _TCHAR *element)
{
    HKEY environmentKey;
    _TCHAR *path;
    _TCHAR *absElement;
    const _TCHAR *updatedPath;

    environmentKey = openRegistryKey(HKEY_CURRENT_USER, REGISTRY_ENVIRONMENT_KEY_NAME);
    path = getRegistryStringData(environmentKey, REGISTRY_USER_PATH_VALUE_NAME);
    absElement = getAbsolutePath(element);
    updatedPath = addElementToPath(path, absElement);
    if (updatedPath == path) {
        _tprintf(_T("Directory %ls already exists in user Path in the registry.\n"), absElement);
    } else {
        _tprintf(_T("Appending %ls to user Path in the registry.\n"), absElement);
        setRegistryStringData(environmentKey, REGISTRY_USER_PATH_VALUE_NAME, updatedPath);
    }
    closeRegistryKey(environmentKey, REGISTRY_ENVIRONMENT_KEY_NAME);
}

HKEY openRegistryKey(HKEY parentKey, const _TCHAR *keyName) {
    LSTATUS openResult;
    HKEY openedKey;
    openResult = RegOpenKeyExW(parentKey, keyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &openedKey);
    if (openResult != ERROR_SUCCESS) {
        writeLastError(openResult, _T("Failed to open registry key"), keyName);
        exit(EXIT_FAILURE);
    }
    return openedKey;
}

void closeRegistryKey(HKEY key, const _TCHAR *keyName) {
    LSTATUS result;
    result = RegCloseKey(key);
    if (result != ERROR_SUCCESS) {
        writeLastError(result, _T("Failed to close registry key"), keyName);
        exit(EXIT_FAILURE);
    }
}

void setRegistryStringData(HKEY key, const _TCHAR *valueName, const _TCHAR *data) {
    size_t sizeInBytes;
    LSTATUS status;
    sizeInBytes = sizeof(_TCHAR) * (_tcslen(data) + 1);
    status = RegSetValueExW(key, valueName, 0, REG_EXPAND_SZ, (const BYTE *) data, (DWORD) sizeInBytes);
    if (status == ERROR_SUCCESS) {
        wprintf(L"Restart your command line window to enable the updated Path.\n");
    } else {
        writeLastError(status, _T("Failed to set registry data for value name"), valueName);
    }
}

TCHAR *getRegistryStringData(HKEY key, const TCHAR *valueName)
{
    LSTATUS status;
    DWORD dataType;
    DWORD requiredBufferSizeInBytes = 0;
    DWORD retrievedDataSize = 0;
    TCHAR *data = NULL;

    /* Request the size of the buffer required to store the data retrieved from this registry value. */
    status = RegGetValue(key, NULL, valueName, RRF_RT_ANY, NULL, NULL, &requiredBufferSizeInBytes);
    if (status == ERROR_MORE_DATA || status == ERROR_SUCCESS) {
        /* This is what we wanted to happen. Now we know the required size. */
        data = (TCHAR *) GC_MALLOC(requiredBufferSizeInBytes);
        if (data == NULL) {
            writeError(errno, _T("Failed to allocate memory for registry data"), valueName);
            exit(EXIT_FAILURE);
        }
        retrievedDataSize = requiredBufferSizeInBytes;
        status = RegGetValue(key, NULL, valueName, RRF_RT_ANY, &dataType, data, &retrievedDataSize);
        if (status != ERROR_SUCCESS) {
            writeLastError(status, _T("Failed to get registry data for value"), valueName);
            exit(EXIT_FAILURE);
        }
    } else {
        writeLastError(status, _T("Failed to get required size of buffer for registry data"), valueName);
        exit(EXIT_FAILURE);
    }
    return data;
}

static const _TCHAR *addElementToPath(const _TCHAR *path, const _TCHAR *element) {
    const _TCHAR *updatedPath;
    if (stringContains(path, element)) {
        /* It's already in dare. */
        updatedPath = path;
    } else {
        /* The elements is not in the Path yet. */
        if (endsWithChar(path, ';')) {
            updatedPath = concat3(path, element, _T(";"));
        } else {
            updatedPath = concat4(path, _T(";"), element, _T(";"));
        }
    }
    return updatedPath;
}
