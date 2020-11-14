/***************************************************************************
 *
 * du.cpp
 * Copyright (c) 2004 Bill Chatfield
 * All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

/*
 * NOTES:
 * <ol>
 * <li>
 * Win32 needs to be used rather than things like UWP because UWP limits
 * the target platforms to Windows 10. It would not work on ReactOS or
 * Wine, for example, if UWP were used.
 * </li>
 * <li>
 * The recursive calculation of file sizes is very resource intensive so
 * Win32 is also needed because it is the fastest API for Windows.
 * </li>
 * </ol>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <tchar.h>
#include "build-number.h"
#include "version.h"
#include "string-utils.h"
#include "error-handling.h"

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

#ifdef UNICODE
#define EXTENDED_LENGTH_PATH_PREFIX _T("\\\\?\\")
#else
#define EXTENDED_LENGTH_PATH_PREFIX ""
#endif

#define DEFAULT_PATH _T(".")
#define DIR_SEPARATOR _T("\\")
#define FIND_ALL_PATTERN _T("\\*")

static void usage();
static void version();
static int removeElement(int argc, TCHAR* argv[], int elementNumber);
static int setSwitches(int argc, TCHAR* argv[]);
static void printFileSize(const TCHAR* absolutePath, const TCHAR *origPathPtr, unsigned long size);
static unsigned long getSize(const TCHAR* fileName, const TCHAR *origPathPtr, bool isTopLevel);
static unsigned long getSizeOfDirectory(const TCHAR* name, const TCHAR *origPathPtr, bool isTopLevel);
static unsigned long getSizeOfRegularFile(const TCHAR* name, const TCHAR *origPathPtr, bool isTopLevel);
static void displaySizesOfMatchingFiles(const TCHAR* glob, const TCHAR *origPathPtr, bool isTopLevel);
static TCHAR *prefixForExtendedLengthPath(const TCHAR *path);
static TCHAR *buildPath(const TCHAR *left, const TCHAR *right);
static TCHAR *getAbsolutePath(const TCHAR *path const TCHAR **origPathPtr);

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
TCHAR *programName;

int _tmain(int argc, TCHAR* argv[])
{
    int i;
    const TCHAR *argument;
    int nonSwitchArgumentCount;
    TCHAR *absolutePath = NULL;
    TCHAR *origPathPtr;

    programName = argv[0];
    nonSwitchArgumentCount = setSwitches(argc, argv);
    if (nonSwitchArgumentCount > 1) {
        for (i = 1; i < nonSwitchArgumentCount; i++) {
            argument = argv[i];
            absolutePath = getAbsolutePath(argument, &origPathPtr);
            if (isGlob(argument)) {  /* Will be true if compiled with Visual C++, but not with GCC. */
                displaySizesOfMatchingFiles(absolutePath, origPathPtr, true);
            }
            else {
                getSize(absolutePath, origPathPtr, true);
            }
            free(absolutePath);
        }
    }
    else {
        absolutePath = getAbsolutePath(DEFAULT_PATH, &origPathPtr);
        getSize(origPathPtr, absolutePath, true);
        free(absolutePath);
    }
    return EXIT_SUCCESS;
}

void usage()
{
    _putts(_T("Usage: du [OPTION]... [FILE]..."));
    _putts(_T("Summarize disk usage of each FILE, recursively for directories."));
    _putts(_T("File and directory sizes are written in kilobytes."));
    _putts(_T("1 kilobyte = 1024 bytes"));
    _putts(_T(""));
    _putts(_T("-a          write counts for all files, not just directories"));
    _putts(_T("-b          print size in bytes"));
    _putts(_T("-s          display only a total for each argument"));
    _putts(_T("-?, --help  display this help and exit"));
    _putts(_T("--version   output version information and exit"));
    _putts(_T(""));
    _putts(_T("Example: du -s *"));
    _putts(_T(""));
    _putts(_T("Report bugs https://github.com/gungwald/du"));
}

void version()
{
    _tprintf(_T("du for Windows - Version %d.%d.%d.%d\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD);
    _putts(_T(VER_COPYRIGHT_STR));
    _putts(_T("Distributed under the GNU General Public License v3."));
    _putts(_T(""));
    _putts(_T("This du is written to the native Win32 API so that"));
    _putts(_T("it will be as fast as possible.  It does not depend"));
    _putts(_T("on any special UNIX emulation libraries.  It also"));
    _putts(_T("displays correct values for file and directory sizes"));
    _putts(_T("unlike some other versions of du ported from UNIX-like"));
    _putts(_T("operating systems."));
}

int removeElement(int argc, TCHAR* argv[], int elementNumber)
{
    int i;

    for (i = elementNumber; i < argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    return i;
}

int setSwitches(int argc, TCHAR* argv[])
{
    int i = 0;
    int newArgumentCount = 0;

    newArgumentCount = argc;
    while (i < newArgumentCount) {
        if (_tcscmp(argv[i], _T("/?")) == 0 || _tcscmp(argv[i], _T("-?")) == 0 || _tcscmp(argv[i], _T("--help")) == 0) {
            usage();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _T("--version")) == 0) {
            version();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _T("/a")) == 0 || _tcscmp(argv[i], _T("-a")) == 0) {
            displayRegularFilesAlso = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _T("/b")) == 0 || _tcscmp(argv[i], _T("-b")) == 0) {
            displayBytes = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _T("/s")) == 0 || _tcscmp(argv[i], _T("-s")) == 0) {
            summarize = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else {
            i++;
        }
    }
    if (displayRegularFilesAlso && summarize) {
        _ftprintf(stderr, _T("%s: ERROR with arguments: cannot both summarize and show all entries\n"), programName);
        exit(EXIT_FAILURE);
    }
    return newArgumentCount;
}

void printFileSize(const TCHAR *absolutePath, const TCHAR *origPathPtr, unsigned long size)
{
    if (!displayBytes) {
        if (size > 0) {
            size = size / ((unsigned long)(1024.0 + 0.5));  /* Convert to KB and round */
            if (size == 0) {
                size = 1;  /* Don't allow zero to display if there are bytes in the file */
            }
        }
    }
    _tprintf(_T("%-7lu %s\n"), size, origPathPtr);
}

void displaySizesOfMatchingFiles(const TCHAR *absSearchPattern, const TCHAR *origPathPtr, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool moreMatchesForThisArgument = true;
    DWORD lastError;
    const TCHAR *baseName;
    TCHAR *absSearchDirectory;
    TCHAR *absChildPath;

    findHandle = FindFirstFile(searchPattern, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), _T("failed to get handle for search pattern"), absSearchPattern);
    }
    else {
        absSearchDirectory = dirname(absSearchPattern);
        while (moreMatchesForThisArgument) {
            baseName = fileProperties.cFileName;
            if (_tcscmp(baseName, _T(".")) != 0 && _tcscmp(baseName, _T("..")) != 0) {
                absChildPath = buildPath(absSearchDirectory, baseName);
                getSize(path, origPathPtr, true);
                free(absChildPath);
            }
            if (!FindNextFile(findHandle, &fileProperties)) {
                lastError = GetLastError();
                if (lastError == ERROR_NO_MORE_FILES) {
                    moreMatchesForThisArgument = false;
                }
                else {
                    writeLastError(lastError, _T("failed to get next file matching pattern"), absSearchPattern);
                }
            }
        }
        free(absSearchDirectory);
        FindClose(findHandle);  /* Only close it if it got opened successfully */
    }
}

unsigned long getSizeOfDirectory(const TCHAR *absDirectoryPath, const TCHAR *origPathPtr, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool moreDirectoryEntries = true;
    TCHAR *absSearchPattern;
    const TCHAR *childBasename;
    TCHAR *absChildPath;
    DWORD lastError;
    unsigned long size = 0;

    asbSearchPattern = buildPath(absDirectoryPath, _T("*"));
    if (absSearchPattern != NULL) {
        findHandle = FindFirstFile(absSearchPattern, &fileProperties)
        if (fileHandle == INVALID_HANDLE_VALUE) {
            writeLastError(GetLastError(), _T("failed to get handle for file search pattern"), absSearchPattern);
        }
        else {
            while (moreDirectoryEntries) {
                childBasename = fileProperties.cFileName;
                if (_tcscmp(childBasename, _T(".")) != 0 && _tcscmp(childBasename, _T("..")) != 0) {
                    if ((absChildPath = buildPath(directoryName, childBasename)) != NULL) {
                        size += getSize(absChildPath, origPathPtr, false);  /* RECURSION */
                        free(absChildPath);
                    }
                }
                if (!FindNextFile(findHandle, &fileProperties)) {
                    if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
                        moreDirectoryEntries = false;
                    }
                    else {
                        writeLastError(lastError, _T("failed to get next file search results"), absSearchPattern);
                    }
                }
            }
            FindClose(findHandle);  /* Only close it if it got opened successfully */
            if (!summarize || isTopLevel) {
                printFileSize(absDirectoryPath, origPathPtr, size);
            }
        }
        free(absSearchPattern);
    }
    return size;
}

unsigned long getSizeOfRegularFile(const TCHAR *absolutePath, const TCHAR *origPathPtr, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    unsigned long size = 0;
    unsigned long multiplier;
    unsigned long maxDWORD;

    findHandle = FindFirstFile(absolutePath, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), _T("failed to get handle for file"), absolutePath);
    }
    else {
        maxDWORD = (unsigned long) MAXDWORD;  /* Avoid Visual C++ 4.0 warning */
        multiplier = maxDWORD + 1UL;
        size = fileProperties.nFileSizeHigh * multiplier + fileProperties.nFileSizeLow;
        FindClose(findHandle);
        if (displayRegularFilesAlso || isTopLevel) {
            printFileSize(absolutePath, origPathPtr, size);
        }
    }
    return size;
}

unsigned long getSize(const TCHAR *absolutePath, const TCHAR *origPathPtr, bool isTopLevel)
{
    DWORD fileAttributes;
    unsigned long size = 0;

    fileAttributes = GetFileAttributes(absolutePath);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        writeLastError(GetLastError(), _T("failed to get attributes for file"), absolutePath);
    }
    else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        size = getSizeOfDirectory(absolutePath, origPathPtr, isTopLevel);    /* RECURSION */
    }
    else {
        size = getSizeOfRegularFile(absolutePath, origPathPtr, isTopLevel);
    }
    return size;
}

/* Result must be freed. */
TCHAR *prefixForExtendedLengthPath(const TCHAR *path)
{
    return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

/* Result must be freed. */
TCHAR *buildPath(const TCHAR *left, const TCHAR *right)
{
    TCHAR *path;

    if (_tcscmp(left, _T(".")) == 0) {
        path = _tcsdup(right);    /* Drop the . for current directory because it causes problems */
    }
    else {
        path = concat3(left, DIR_SEPARATOR, right);
    }
    return path;
}

/* Result must be freed. */
TCHAR *getAbsolutePath(const TCHAR *path, const TCHAR **origPathPtr)
{
    TCHAR *absolutePath = NULL;
    TCHAR *prefixedPath;
    DWORD requiredBufferSize;
    DWORD returnedPathLength;

    prefixedPath = prefixForExtendedLengthPath(path);

    /* Ask for the size of the buffer needed to hold the absolute path. */
    requiredBufferSize = GetFullPathName(prefixedPath, 0, NULL, NULL);
    if (requiredBufferSize == 0) {
        writeLastError(GetLastError(), _T("failed to get buffer size required for file name"), prefixedPath);
        exit(EXIT_FAILURE);
    }
    else {
        absolutePath = (TCHAR *) malloc(requiredBufferSize * sizeof(TCHAR));
        if (absolutePath == NULL) {
            writeError(errno, _T("memory allocation failed for absolute path of"), prefixedPath);
            exit(EXIT_FAILURE);
        }
        else {
            returnedPathLength = GetFullPathName(prefixedPath, requiredBufferSize, absolutePath, NULL);
            if (returnedPathLength == 0) {
                writeLastError(GetLastError(), _T("failed to get full path name for "), prefixedPath);
            }
            else if (returnedPathLength >= requiredBufferSize) {
                writeLastError(GetLastError(), _T("buffer was not big enough for "), prefixedPath);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (absolutePath != NULL) {
        *origPathPtr = strstr(absolutePath, path)
        if (*origPathPtr == NULL) {
            _ftprintf(stderr, _T("can't find original path %s in absolute path %s\n"), path, absolutePath);
        }
    }
    return absolutePath;
}
