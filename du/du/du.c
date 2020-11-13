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

#ifdef _UNICODE
#define EXTENDED_LENGTH_PATH_PREFIX _TEXT("\\\\?\\")
#else
#define LONG_PATH_ENABLER ""
#endif

#define DIR_SEPARATOR _TEXT("\\")
#define FIND_ALL_PATTERN _TEXT("\\*")

static void usage();
static void version();
static int removeElement(int argc, _TCHAR* argv[], int elementNumber);
static int setSwitches(int argc, _TCHAR* argv[]);
static void printFileSize(const _TCHAR* fileName, unsigned long size);
static unsigned long getSize(const _TCHAR* fileName, bool isTopLevel);
static unsigned long getSizeOfDirectory(const _TCHAR* name, bool isTopLevel);
static unsigned long getSizeOfRegularFile(const _TCHAR* name, bool isTopLevel);
static void displaySizesOfMatchingFiles(const _TCHAR* glob, bool isTopLevel);
static _TCHAR *prefixForExtendedLengthPath(const _TCHAR *path);
static _TCHAR *buildPath(const _TCHAR *left, const _TCHAR *right);
static _TCHAR *getAbsolutePath(const _TCHAR *path);
static _TCHAR *getExtendedAbsolutePath(const _TCHAR *path);
static bool isArgumentAbsolutePath(const _TCHAR *path);

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
bool argumentIsAbsolutePath = false;
_TCHAR* programName;

int _tmain(int argc, _TCHAR* argv[])
{
    int i;
    const _TCHAR *argument;
    int nonSwitchArgumentCount;
    _TCHAR *absolutePath = NULL;
    _TCHAR *prefixedPath = NULL;

    programName = argv[0];
    nonSwitchArgumentCount = setSwitches(argc, argv);
    if (nonSwitchArgumentCount > 1) {
        for (i = 1; i < nonSwitchArgumentCount; i++) {
            argument = argv[i];
            argumentIsAbsolutePath = isArgumentAbsolutePath(argument);
            absolutePath = getAbsolutePath(argument);
            prefixedPath = prefixForExtendedLengthPath(absolutePath);
            if (isGlob(argument)) {  /* Will be true if compiled with Visual C++, but not with GCC. */
                displaySizesOfMatchingFiles(prefixedPath, true);
            }
            else {
                getSize(prefixedPath, true);
            }
            free(prefixedPath);
            free(absolutePath);
        }
    }
    else {
        absolutePath = getAbsolutePath(_TEXT("."));
        prefixedPath = prefixForExtendedLengthPath(absolutePath);
        getSize(prefixedPath, true);
        free(prefixedPath);
        free(absolutePath);
    }
    return EXIT_SUCCESS;
}

void usage()
{
    _putts(_TEXT("Usage: du [OPTION]... [FILE]..."));
    _putts(_TEXT("Summarize disk usage of each FILE, recursively for directories."));
    _putts(_TEXT("File and directory sizes are written in kilobytes."));
    _putts(_TEXT("1 kilobyte = 1024 bytes"));
    _putts(_TEXT(""));
    _putts(_TEXT("-a          write counts for all files, not just directories"));
    _putts(_TEXT("-b          print size in bytes"));
    _putts(_TEXT("-s          display only a total for each argument"));
    _putts(_TEXT("-?, --help  display this help and exit"));
    _putts(_TEXT("--version   output version information and exit"));
    _putts(_TEXT(""));
    _putts(_TEXT("Example: du -s *"));
    _putts(_TEXT(""));
    _putts(_TEXT("Report bugs https://github.com/gungwald/du"));
}

void version()
{
    _tprintf(_TEXT("du for Windows - Version %d.%d.%d.%d\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD);
    _putts(_TEXT(VER_COPYRIGHT_STR));
    _putts(_TEXT("Distributed under the GNU General Public License v3."));
    _putts(_TEXT(""));
    _putts(_TEXT("This du is written to the native Win32 API so that"));
    _putts(_TEXT("it will be as fast as possible.  It does not depend"));
    _putts(_TEXT("on any special UNIX emulation libraries.  It also"));
    _putts(_TEXT("displays correct values for file and directory sizes"));
    _putts(_TEXT("unlike some other versions of du ported from UNIX-like"));
    _putts(_TEXT("operating systems."));
}

int removeElement(int argc, _TCHAR* argv[], int elementNumber)
{
    int i;

    for (i = elementNumber; i < argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    return i;
}

int setSwitches(int argc, _TCHAR* argv[])
{
    int i = 0;
    int newArgumentCount = 0;

    newArgumentCount = argc;
    while (i < newArgumentCount) {
        if (_tcscmp(argv[i], _TEXT("/?")) == 0 || _tcscmp(argv[i], _TEXT("-?")) == 0 || _tcscmp(argv[i], _TEXT("--help")) == 0) {
            usage();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _TEXT("--version")) == 0) {
            version();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _TEXT("/a")) == 0 || _tcscmp(argv[i], _TEXT("-a")) == 0) {
            displayRegularFilesAlso = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _TEXT("/b")) == 0 || _tcscmp(argv[i], _TEXT("-b")) == 0) {
            displayBytes = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _TEXT("/s")) == 0 || _tcscmp(argv[i], _TEXT("-s")) == 0) {
            summarize = true;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else {
            i++;
        }
    }
    if (displayRegularFilesAlso && summarize) {
        _ftprintf(stderr, _TEXT("%s: ERROR with arguments: cannot both summarize and show all entries\n"), programName);
        exit(EXIT_FAILURE);
    }
    return newArgumentCount;
}

void printFileSize(const _TCHAR* fileName, unsigned long size)
{
    if (!displayBytes) {
        if (size > 0) {
            size = size / ((unsigned long)(1024.0 + 0.5));  /* Convert to KB and round */
            if (size == 0) {
                size = 1;  /* Don't allow zero to display if there are bytes in the file */
            }
        }
    }
	_tprintf(_TEXT("%-7lu %s\n"), size, fileName);
}

void displaySizesOfMatchingFiles(const _TCHAR* searchPattern, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool moreMatchesForThisArgument = true;
    DWORD lastError;
    const _TCHAR *fileName;
    _TCHAR* searchDirectory;
    _TCHAR *path;

    findHandle = FindFirstFile(searchPattern, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), _TEXT("failed to get handle for search pattern"), searchPattern);
    }
    else {
        searchDirectory = dirname(searchPattern);
        while (moreMatchesForThisArgument) {
            fileName = fileProperties.cFileName;
            if (_tcscmp(fileName, _TEXT(".")) != 0 && _tcscmp(fileName, _TEXT("..")) != 0) {
                path = concat3(searchDirectory, DIR_SEPARATOR, fileName);   /* Make sure the file can be found later. */
                getSize(path, true);
                free(path);
            }
            if (!FindNextFile(findHandle, &fileProperties)) {
                lastError = GetLastError();
                if (lastError == ERROR_NO_MORE_FILES) {
                    moreMatchesForThisArgument = false;
                }
                else {
                    writeLastError(lastError, _TEXT("failed to get next file matching pattern"), searchPattern);
                }
            }
        }
        free(searchDirectory);
        FindClose(findHandle);  /* Only close it if it got opened successfully */
    }
}

unsigned long getSizeOfDirectory(const _TCHAR* directoryName, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool moreDirectoryEntries = true;
    _TCHAR* searchPattern;
    const _TCHAR* childFileName;
    _TCHAR* childPath;
    DWORD lastError;
    unsigned long size = 0;

    searchPattern = buildPath(directoryName, _TEXT("*"));
    if (searchPattern != NULL) {
        if ((findHandle = FindFirstFile(searchPattern, &fileProperties)) == INVALID_HANDLE_VALUE) {
            writeLastError(GetLastError(), _TEXT("failed to get handle for file search pattern"), searchPattern);
        }
        else {
            while (moreDirectoryEntries) {
                childFileName = fileProperties.cFileName;
                if (_tcscmp(childFileName, _TEXT(".")) != 0 && _tcscmp(childFileName, _TEXT("..")) != 0) {
                    if ((childPath = buildPath(directoryName, childFileName)) != NULL) {
                        size += getSize(childPath, false);  /* RECURSION */
                        free(childPath);
                    }
                }
                if (!FindNextFile(findHandle, &fileProperties)) {
                    if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
                        moreDirectoryEntries = false;
                    }
                    else {
                        writeLastError(lastError, _TEXT("failed to get next file search results"), searchPattern);
                    }
                }
            }
            FindClose(findHandle);  /* Only close it if it got opened successfully */
            if (!summarize || isTopLevel) {
                printFileSize(directoryName, size);
            }
        }
        free(searchPattern);
    }
    return size;
}

unsigned long getSizeOfRegularFile(const _TCHAR* name, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    unsigned long size = 0;
    unsigned long multiplier;
    unsigned long maxDWORD;

    findHandle = FindFirstFile(name, &fileProperties);
    if (findHandle == INVALID_HANDLE_VALUE) {
        writeLastError(GetLastError(), _TEXT("failed to get handle for file"), name);
    }
    else {
        maxDWORD = (unsigned long)MAXDWORD;  /* Avoid Visual C++ 4.0 warning */
        multiplier = maxDWORD + 1UL;
        size = fileProperties.nFileSizeHigh * multiplier + fileProperties.nFileSizeLow;
        FindClose(findHandle);
        if (displayRegularFilesAlso || isTopLevel) {
            printFileSize(name, size);
        }
    }
    return size;
}

unsigned long getSize(const _TCHAR* path, bool isTopLevel)
{
    DWORD fileAttributes;
    unsigned long size = 0;
    _TCHAR *prefixedPath;

    fileAttributes = GetFileAttributes(path);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        writeLastError(GetLastError(), _TEXT("failed to get attributes for file"), path);
    }
    else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        size = getSizeOfDirectory(path, isTopLevel);    /* RECURSION */
    }
    else {
        size = getSizeOfRegularFile(path, isTopLevel);
    }
    return size;
}

/* Result must be freed. */
_TCHAR *prefixForExtendedLengthPath(const _TCHAR *path)
{
    return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

/* Result must be freed. */
_TCHAR *buildPath(const _TCHAR *left, const _TCHAR *right)
{
    _TCHAR *path;

    if (_tcscmp(left, _TEXT(".")) == 0) {
        path = _tcsdup(right);    /* Drop the . for current directory because it causes problems */
    }
    else {
        path = concat3(left, DIR_SEPARATOR, right);
    }
    return path;
}

/* Result must be freed. */
_TCHAR *getAbsolutePath(const _TCHAR *path)
{
    _TCHAR *absolutePath;
    DWORD requiredBufferSize;
    DWORD returnedPathLength;

    /* Ask for the size of the buffer needed to hold the absolute path. */
    requiredBufferSize = GetFullPathName(path, 0, NULL, NULL);
    if (requiredBufferSize == 0) {
        writeLastError(GetLastError(), _TEXT("failed to get buffer size required for file name"), path);
        exit(EXIT_FAILURE);
    }
    else {
        absolutePath = (_TCHAR *) malloc(requiredBufferSize * sizeof(_TCHAR));
        if (absolutePath == NULL) {
			writeError(errno, _TEXT("Memory allocation failed for absolute path of"), path);
			exit(EXIT_FAILURE);
        }
        else {
            returnedPathLength = GetFullPathName(path, requiredBufferSize, absolutePath, NULL);
            if (returnedPathLength == 0) {
                writeLastError(GetLastError(), _TEXT("failed to get full path name for "), path);
            }
            else if (returnedPathLength >= requiredBufferSize) {
				writeLastError(GetLastError(), _TEXT("buffer was not big enough for "), path);
				exit(EXIT_FAILURE);
            }
        }
    }
    return absolutePath;
}

_TCHAR *getExtendedAbsolutePath(const _TCHAR *path)
{
    _TCHAR *prefixedPath;
    _TCHAR *absolutePath;

    prefixedPath = prefixForExtendedLengthPath(path);
    absolutePath = getAbsolutePath(prefixedPath);
    free(prefixedPath);
    return absolutePath;
}

bool isArgumentAbsolutePath(const _TCHAR *path)
{
    return path[0] == _TEXT('\\') || _tcschr(path, _TEXT(':')) != NULL;
}
