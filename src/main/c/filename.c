#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "filename.h"
#include "string.h"
#include "error.h"
#include "trace.h"

extern wchar_t *makeExtendedLengthPath(const wchar_t *path);
extern wchar_t *makeNormalPath(const wchar_t *path);
extern bool isExtendedLengthPath(const wchar_t *path);
static wchar_t *slashToBackslash(const wchar_t *path);
static wchar_t *skipPrefix(wchar_t *path);

/* Result should be freed. */
static wchar_t *slashToBackslash(const wchar_t *path)
{
    /* Make sure all separators are backslashes. */
    return replaceAll(_tcsdup(path), _TEXT('/'), _TEXT('\\'));
}

/* Result must be freed. */
wchar_t *getAbsolutePath(const wchar_t *path)
{
    wchar_t *absolutePath = NULL;
    DWORD reqSize;
    DWORD returnedLen;

    /* Ask for the size of the buffer needed to hold the absolute path. */
    reqSize = GetFullPathName(path, 0, NULL, NULL);
    if (reqSize == 0) {
        writeLastError(GetLastError(), L"failed to get required buf size", path);
        exit(EXIT_FAILURE);
    } else {
        absolutePath = (wchar_t *) malloc(reqSize * sizeof(wchar_t));
        if (absolutePath == NULL) {
            writeError(errno, _T("memory alloc failed for abs path of"), path);
            exit(EXIT_FAILURE);
        } else {
            returnedLen = GetFullPathName(path, reqSize, absolutePath, NULL);
            if (returnedLen == 0) {
                writeLastError(GetLastError(), L"failed to get full path", path);
                exit(EXIT_FAILURE);
            } else if (returnedLen >= reqSize) {
                writeLastError(GetLastError(), L"buffer not big enough", path);
                exit(EXIT_FAILURE);
            }
        }
    }
    return absolutePath;
}

/* Result must be freed. */
wchar_t *makeExtendedLengthPath(const wchar_t *path)
{
    return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

/* Result must be freed. */
wchar_t *getParentPath(const wchar_t *path)
{
    wchar_t *parent;
    wchar_t *lastBackslashPointer;
    const wchar_t *absPath;
    size_t parentSize;

    if (isAbsolutePath(path)) {
        absPath = path;
    } else {
        absPath = getAbsolutePath(path);
    }
    lastBackslashPointer = wcsrchr(absPath, L'\\');
    parentSize = lastBackslashPointer - absPath + 1;
    parent = (wchar_t *) malloc(parentSize * sizeof(wchar_t));
    strncpy(parent, parentSize, absPath);
    if (absPath != path) {
        free(absPath);
    }
    return parent;
}

/* Returns pointer which should not be freed. */
wchar_t *getSimpleName(const wchar_t *path)
{
    wchar_t *lastBackslash;
    wchar_t *simpleName;

    lastBackslash = wcsrchr(path, L'\\');
    if (lastBackslash == NULL)
        simpleName = path;
    else
        simpleName = lastBackslash + 1;
    return simpleName;
}

/* Returns pointer which should not be freed. */
wchar_t *skipPrefix(wchar_t *path)
{
    size_t prefixLength;
    wchar_t *result;

    prefixLength = wcslen(EXTENDED_LENGTH_PATH_PREFIX);
    if (prefixLength > 0) {
        if (wcsncmp(path, EXTENDED_LENGTH_PATH_PREFIX, prefixLength) == 0)
            result = path + prefixLength;
        else
            result = path;
    } else
        result = path;
    return result;
}

unsigned long getFileSize(wchar_t *path)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    unsigned long size = 0;
    unsigned long multiplier;
    unsigned long maxDWORD;

    findHandle = FindFirstFile(path, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), L"Failed to get handle for file", path);
    } else {
        maxDWORD = (unsigned long) MAXDWORD; /* Avoid Visual C++ 4.0 warning */
        multiplier = maxDWORD + 1UL;
        size = fileProperties.nFileSizeHigh * multiplier + fileProperties.nFileSizeLow;
        FindClose(findHandle);
    }
    return size;
}

List *listFiles(const wchar_t *path)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    wchar_t *search;
    bool moreDirectoryEntries;
    wchar_t *entry;
    List *files;
    DWORD lastError;

    files = initList();
    if (isGlob(path)) {
        search = path;
    } else {
        search = concat3(path, DIR_SEPARATOR, L"*");

    }
    findHandle = FindFirstFile(search, &fileProperties);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), L"Failed to get handle for pattern", search);
    } else {
        moreDirectoryEntries = true;
        while (moreDirectoryEntries) {
            entry = fileProperties.cFileName;
            if (wcscmp(entry, L".") != 0 && wcscmp(entry, L"..") != 0) {
                    appendListNode(files, entry);
                }
            }
            if (!FindNextFile(findHandle, &fileProperties)) {
                if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
                    moreDirectoryEntries = false;
                } else {
                    writeLastError(lastError, L"Failed to get next results", search);
                }
            }
        }
        FindClose(findHandle); /* Only close it if it got opened successfully */
    }
    free(search);
    return files;
}

enum FileType getFileType(wchar_t *path)
{
    DWORD fileAttributes;
    enum FileType = type;

    if (isGlob(path)) {
        type = FILETYPE_GLOB;
    } else {
        fileAttributes = GetFileAttributes(path);
        if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
            writeLastError(GetLastError(), L"Failed to get file attributes", path);
        } else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            type = FILETYPE_DIRECTORY;
        } else {
            type = FILETYPE_FILE;
        }
    }
    return type;
}

bool isFile(wchar_t *path)
{
    return getFileType(path) == FILETYPE_FILE;
}

bool isDirectory(File *f)
{
    return getFileType(path) == FILETYPE_DIRECTORY;
}

bool isGlobPattern(File *f)
{
    return getFileType(path) == FILETYPE_GLOB;
}

