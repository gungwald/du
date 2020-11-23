// install.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>                 /* EXIT_FAILURE */
#include <stdbool.h>                /* bool, true, false */
#include <windows.h>
#include <tchar.h>
#include <lmerr.h>
#include <conio.h>
#include "../du/error-handling.h"
#include "../du/string-utils.h"
#include "../du/path.h"

#define STRING_CAPACITY 1024
#define FILE_TO_INSTALL _T("du.exe")
#define PATH_REG_VALUE _T("Path")

static TCHAR *getRegistryStringValueForUpdate(HKEY key, const TCHAR *value);
static bool fileExists(TCHAR *path);
static void exitHandler(void);
TCHAR *programName;

int _tmain(int argc, TCHAR *argv[])
{
    HKEY environmentKey;
    LSTATUS status;
    TCHAR *path;
    TCHAR userProfile[STRING_CAPACITY];
    Path *installDir;
    Path *homeDir;
    Path *installTargetFile;
    TCHAR *updatedPath;
    DWORD sizeInBytes;

    programName = argv[0];
    atexit(exitHandler);

    if (GetEnvironmentVariable(_T("USERPROFILE"), userProfile, STRING_CAPACITY) == 0) {
        writeLastError(GetLastError(), _T("Failed to get value of environment variable"), _T("USERPROFILE"));
        exit(EXIT_FAILURE);
    }

    homeDir = path_Init(userProfile);
    installDir = path_Append(homeDir, _T("bin"));
    installTargetFile = path_Append(installDir, FILE_TO_INSTALL);

    if (!fileExists(installDir->absolute)) {
        _tprintf(_T("Creating directory %s.\n"), path_GetAbsolute(installDir));
        if (!CreateDirectory(installDir->absolute, NULL)) {
            writeLastError(GetLastError(), _T("Failed to create install directory"), installDir->absolute);
            exit(EXIT_FAILURE);
        }
    }
    else {
        _tprintf(_T("Directory %s already exists.\n"), path_GetAbsolute(installDir));
    }

    _tprintf(_T("Copying %s to %s\n"), FILE_TO_INSTALL, path_GetAbsolute(installDir));
    if (!CopyFile(FILE_TO_INSTALL, installTargetFile->absolute, FALSE)) {
		writeLastError(GetLastError(), _T("Failed to copy file"), installTargetFile->absolute);
		exit(EXIT_FAILURE);
    }

    status = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Environment"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &environmentKey);
    if (status != ERROR_SUCCESS) {
        writeLastError(status, _T("Failed to open registry key"), _T("Environment"));
        exit(EXIT_FAILURE);
    }

	path = getRegistryStringValueForUpdate(environmentKey, PATH_REG_VALUE);
    if (_tcsstr(path, path_GetAbsolute(installDir)) == NULL) {
        /* Our installDir is not in the Path yet. */
        _tprintf(_T("Appending %s to user Path in the registry.\n"), path_GetAbsolute(installDir));
        if (path[_tcslen(path) - 1] == ';') {
            updatedPath = concat(path, path_GetAbsolute(installDir));
        }
        else {
            updatedPath = concat3(path, _T(";"), path_GetAbsolute(installDir));
        }
        sizeInBytes = sizeof(TCHAR) * (_tcslen(updatedPath) + 1);
        status = RegSetValueEx(environmentKey, PATH_REG_VALUE, 0, REG_EXPAND_SZ, (const BYTE *) updatedPath, sizeInBytes);
        if (status != ERROR_SUCCESS) {
            writeLastError(status, _T("Failed to set registry value"), PATH_REG_VALUE);
        }
        free(updatedPath);
	}
    else {
        _tprintf(_T("Directory %s already exists in user Path in the registry.\n"), path_GetAbsolute(installDir));
    }
	free(path);
    path_Delete(installTargetFile);
    path_Delete(installDir);
    path_Delete(homeDir);
    return EXIT_SUCCESS;
}

void exitHandler(void)
{
    _tprintf(_T("Press any key to exit the install program."));
    _getch();
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
    }
    else {
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
    }
    else {
        fileExists = true;
        FindClose(findHandle);
    }
    return fileExists;
}
