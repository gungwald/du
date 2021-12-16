
#include <stdio.h>
#include <stdlib.h>                 /* EXIT_FAILURE */
#include <stdbool.h>                /* bool, true, false */
#include <windows.h>
#include <tchar.h>
#include <lmerr.h>
#include <conio.h>
#include "../du/error-handling.h"
#include "../du/string-utils.h"
#include "../du/File.h"

#define FILE_TO_INSTALL _T("du.exe")
#define PATH_REG_VALUE _T("Path")

static TCHAR *getRegistryStringValueForUpdate(HKEY key, const TCHAR *value);
static bool fileExists(TCHAR *path);
TCHAR *programName;

int _tmain(int argc, TCHAR *argv[])
{
    HKEY environmentKey;
    LSTATUS status;
    TCHAR *path;
    TCHAR *updatedPath;
    DWORD sizeInBytes;
    File *targetDir;

    programName = argv[0];

    if (argc > 1) {
        targetDir = new_File(argv[1]);
    } else {
        _ftprintf(stderr, _TEXT("Missing command line parameter for [TARGETDIR]"));
        exit(EXIT_FAILURE);
    }

    status = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Environment"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &environmentKey);
    if (status != ERROR_SUCCESS) {
        writeLastError(status, _T("Failed to open registry key"), _T("Environment"));
        exit(EXIT_FAILURE);
    }

    path = getRegistryStringValueForUpdate(environmentKey, PATH_REG_VALUE);
    if (_tcsstr(path, getAbsolutePath(targetDir)) == NULL) {
        /* Our installDir is not in the Path yet. */
        _tprintf(_T("Appending %s to user Path in the registry.\n"), getAbsolutePath(targetDir));
        if (path[_tcslen(path) - 1] == ';') {
            updatedPath = concat(path, getAbsolutePath(targetDir));
        } else {
            updatedPath = concat3(path, _T(";"), getAbsolutePath(targetDir));
        }
        sizeInBytes = sizeof(TCHAR) * (_tcslen(updatedPath) + 1);
        status = RegSetValueEx(environmentKey, PATH_REG_VALUE, 0, REG_EXPAND_SZ, (const BYTE *) updatedPath, sizeInBytes);
        if (status != ERROR_SUCCESS) {
            writeLastError(status, _T("Failed to set registry value"), PATH_REG_VALUE);
        }
        free(updatedPath);
    } else {
        _tprintf(_T("Directory %s already exists in user Path in the registry.\n"), getAbsolutePath(targetDir));
    }
    free(path);
    return EXIT_SUCCESS;
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
        data = (TCHAR *) malloc(requiredBufferSizeInBytes);
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

bool fileExists(TCHAR *path)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool fileExists;

    findHandle = FindFirstFile(path, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        fileExists = false;
    } else {
        fileExists = true;
        FindClose(findHandle);
    }
    return fileExists;
}
