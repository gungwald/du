#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>     /* iswalpha */
#include <gc.h>         /* GC_MALLOC */
#include <windows.h>
#include "filename.h"
#include "string.h"
#include "error.h"
#include "trace.h"
#include "args.h"

/* Turn off security warnings about string functions. The correct amount of space is always allocated. */
#pragma warning(disable : 4996)

enum FileType {
    FILETYPE_DIRECTORY, FILETYPE_FILE, FILETYPE_GLOB, FILETYPE_UNKNOWN
};

static HANDLE open(const wchar_t *path);
static void close(HANDLE h);
static int64_t getAllocatedFileSize(const wchar_t *path);

/* Result should be freed. */
extern wchar_t* slashToBackslash(const wchar_t *path) {
    /* Make sure all separators are backslashes. */
    return replaceAll(_tcsdup(path), _TEXT('/'), _TEXT('\\'));
}

/* Result must be freed. */
wchar_t* getAbsolutePath(const wchar_t *path) {
    wchar_t *absolutePath = NULL;
    DWORD reqSize;
    DWORD returnedLen;

    /* Ask for the size of the buffer needed to hold the absolute path. */
    reqSize = GetFullPathName(path, 0, NULL, NULL);
    if (reqSize == 0) {
        writeLastError(GetLastError(), L"failed to get required buf size",
                path);
        exit(EXIT_FAILURE);
    } else {
        absolutePath = (wchar_t*) GC_MALLOC(reqSize * sizeof(wchar_t));
        if (absolutePath == NULL) {
            writeError(errno, _T("memory alloc failed for abs path of"), path);
            exit(EXIT_FAILURE);
        } else {
            returnedLen = GetFullPathName(path, reqSize, absolutePath, NULL);
            if (returnedLen == 0) {
                writeLastError(GetLastError(), L"failed to get full path",
                        path);
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
wchar_t* getParentPath(const wchar_t *path) {
    wchar_t *parent;
    wchar_t *lastBackslashPointer;
    wchar_t *absPath;
    size_t parentSize;

    if (isAbsolutePath(path)) {
        absPath = _wcsdup(path); /* wcsdup is not part of ISO C ? */
    } else {
        absPath = getAbsolutePath(path);
    }
    lastBackslashPointer = wcsrchr(absPath, L'\\');
    parentSize = lastBackslashPointer - absPath + 1;
    parent = (wchar_t*) GC_MALLOC(parentSize * sizeof(wchar_t));
    if (parent == NULL) {
        writeError(errno, L"Failed to allocate memory for", path);
        exit(EXIT_FAILURE);
    }
    wcsncpy(parent, absPath, parentSize);
    free(absPath);
    return parent;
}

/* Returns pointer which should not be freed. */
const wchar_t* getSimpleName(const wchar_t *path) {
    const wchar_t *lastBackslash;
    const wchar_t *simpleName;

    lastBackslash = wcsrchr(path, L'\\');
    if (lastBackslash == NULL)
        simpleName = path;
    else
        simpleName = lastBackslash + 1;
    return simpleName;
}

/* Returns pointer which should not be freed. */
extern const wchar_t* skipPrefix(wchar_t *path) {
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

int64_t getFileSize(wchar_t *path) {
    HANDLE findHandle;
    WIN32_FIND_DATA fileFindData;
    int64_t size = 0;
    int64_t multiplier;
    int64_t maxDWORD;

    findHandle = FindFirstFile(path, &fileFindData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), L"Failed to get handle for file", path);
    } else {
        if (!displayBytes) {
            size = getAllocatedFileSize(path);
        } else {
            maxDWORD = (int64_t) MAXDWORD; /* Avoid Visual C++ 4.0 warning */
            multiplier = maxDWORD + 1UL;
            size = fileFindData.nFileSizeHigh * multiplier
                    + fileFindData.nFileSizeLow;
        }
        FindClose(findHandle);
    }
    return size;
}

List* listFiles(const wchar_t *path) {
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    const wchar_t *search;
    bool moreDirectoryEntries;
    wchar_t *entry;
    List *files;
    DWORD lastError;
    wchar_t *entryPath;

    files = initList();
    if (isGlob(path)) {
        search = path;
    } else {
        search = buildPath(path, L"*");
    }
    findHandle = FindFirstFile(search, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), L"Failed to get handle for pattern", search);
    } else {
        moreDirectoryEntries = true;
        while (moreDirectoryEntries) {
            entry = fileProperties.cFileName;
            if (wcscmp(entry, L".") != 0 && wcscmp(entry, L"..") != 0) {
                entryPath = buildPath(path, entry);
                appendListItem(&files, entryPath);
            }
            if (!FindNextFile(findHandle, &fileProperties)) {
                if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
                    moreDirectoryEntries = false;
                } else {
                    writeLastError(lastError, L"Failed to get next results",
                            search);
                }
            }
        }
        FindClose(findHandle); /* Only close it if it got opened successfully */
    }
    return files;
}

enum FileType getFileType(const wchar_t *path) {
    DWORD fileAttributes;
    enum FileType type;

    if (isGlob(path)) {
        type = FILETYPE_GLOB;
    } else {
        fileAttributes = GetFileAttributes(path);
        if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
            type = FILETYPE_UNKNOWN;
            writeLastError(GetLastError(), L"Failed to get file attributes", path);
        } else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            type = FILETYPE_DIRECTORY;
        } else {
            type = FILETYPE_FILE;
        }
    }
    return type;
}

bool isFile(const wchar_t *path) {
    return getFileType(path) == FILETYPE_FILE;
}

bool isDirectory(const wchar_t *path) {
    return getFileType(path) == FILETYPE_DIRECTORY;
}

bool isGlobPattern(const wchar_t *path) {
    return getFileType(path) == FILETYPE_GLOB;
}

bool isAbsolutePath(const wchar_t *path) {
    bool isAbsolutePath;
    size_t len;
    len = wcslen(path);
    if ((len >= 3 && path[1] == L':' && path[2] == L'\\' && iswalpha(path[0]))
            || (len >= 4 && path[2] == L':' && path[3] == L'\\'
                    && iswalpha(path[0]) && iswalpha(path[1]))
            || (len >= 1 && path[0] == L'\\'))
        isAbsolutePath = true;
    else
        isAbsolutePath = false;
    return isAbsolutePath;
}

wchar_t* buildPath(const wchar_t *dir, const wchar_t *file) {
    return concat3(dir, DIR_SEPARATOR, file);
}

bool fileExists(wchar_t *path)
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

/* Result must be freed. */
extern wchar_t* makeExtendedLengthPath(const wchar_t *path) {
    return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

extern wchar_t* makeNormalPath(const wchar_t *path) {
    return wcsdup(path + wcslen(EXTENDED_LENGTH_PATH_PREFIX));
}

extern bool isExtendedLengthPath(const wchar_t *path) {
    return startsWith(path, EXTENDED_LENGTH_PATH_PREFIX);
}

static HANDLE open(const wchar_t *path) {
    HANDLE fileHandle;
    wchar_t *extendedPath;

    if (isAbsolutePath(path)) {
        extendedPath =  makeExtendedLengthPath(path);
    } else {
        extendedPath = makeExtendedLengthPath(getAbsolutePath(path));
    }
    fileHandle = CreateFile(
                    extendedPath,                       /* file name */
                    GENERIC_READ,                       /* desired access */
                    FILE_SHARE_READ | FILE_SHARE_WRITE, /* share mode */
                    NULL,                               /* security attributes */
                    OPEN_EXISTING,                      /* disposition */
                    FILE_FLAG_SEQUENTIAL_SCAN,          /* flags */
                    NULL                                /* template file handle */
                 );
    if (fileHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), L"Failed to open file", path);
        exit(EXIT_FAILURE);
    }
    return fileHandle;
}

static void close(HANDLE fileHandle) {
    if (! CloseHandle(fileHandle)) {
        writeLastError(GetLastError(), L"Failed to close file handle", L"unknown");
    }
}

static int64_t getAllocatedFileSize(const wchar_t *path) {
    HANDLE fileHandle;
    FILE_STANDARD_INFO fileStandardInfo;

    fileHandle = open(path);
    if (! GetFileInformationByHandleEx(fileHandle, FileStandardInfo, &fileStandardInfo, sizeof(FILE_STANDARD_INFO))) {
        writeLastError(GetLastError(), L"Failed to get standard file info", path);
        exit(EXIT_FAILURE);
    }
    close(fileHandle);
    return fileStandardInfo.AllocationSize.QuadPart;
}
