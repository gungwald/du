/***************************************************************************
 *
 * du.cpp
 * Copyright (c) 2004, 2021 Bill Chatfield
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
 * <li>Win32 needs to be used rather than things like UWP because UWP limits
 * the target platforms to Windows 10. It would not work on ReactOS or
 * Wine, for example, if UWP were used.</li>
 * <li>The recursive calculation of file sizes is very resource intensive so
 * Win32 is also needed because it is the fastest API for Windows.</li>
 * </ol>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <wchar.h>
#include <gc.h>
#include "filename.h"
#include "string.h"
#include "error.h"
#include "trace.h"
#include "list.h"
#include "args.h"
#include "help.h"
#include "registry.h"

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

#define DEFAULT_PATH _T(".")
#define FIND_ALL_PATTERN _T("\\*")

#define KIBIBYTE 0x400
#define MEBIBYTE 0x100000
#define GIBIBYTE 0x40000000

static void printFileSize(wchar_t *path, int64_t size);
static int64_t calcDiskUsage(wchar_t *path, bool isTopLevel);
static void setup();
static void du(int argc, const wchar_t *argv[]);
static const wchar_t *getEnvironmentVariable(const wchar_t *name);

const wchar_t *programName;

int wmain(int argc, const wchar_t *argv[])
{
    GC_INIT();
    programName = argv[0];
    if (startsWith(getSimpleName(programName), L"du-setup")) {
        setup();
    } else {
        du(argc, argv);
    }
    return EXIT_SUCCESS;
}

static const wchar_t *getEnvironmentVariable(const wchar_t *name)
{
    wchar_t *value = L"";
    DWORD reqSize;
    const DWORD GETENVVAR_FAILURE = 0;
    DWORD win32ApiError;

    if ((reqSize = GetEnvironmentVariable(name, value, 0)) == GETENVVAR_FAILURE) {
        win32ApiError = GetLastError();
        if (win32ApiError == ERROR_ENVVAR_NOT_FOUND) {
            value = L"";
        } else {
            writeLastError(win32ApiError, L"Failed to get size for environment variable", name);
            exit(EXIT_FAILURE);
        }
    }
    if ((value = (wchar_t *) GC_MALLOC(reqSize * sizeof(wchar_t))) == NULL) {
        writeError(errno, L"Failed to allocate memory for environment variable value for", name);
        exit(EXIT_FAILURE);
    }
    if (GetEnvironmentVariable(name, value, reqSize) == GETENVVAR_FAILURE) {
        writeLastError(GetLastError(), L"Failed to get environment variable", name);
        exit(EXIT_FAILURE);
    }
    return value;
}

static void setup()
{
    const wchar_t *homeDir;
    wchar_t *binDir;
    wchar_t *installedProgram;
    const wchar_t *exactProgramName;
    const BOOL CREATEDIRECTORY_FAILURE = FALSE;
    const BOOL COPYFILE_FAILURE = FALSE;

    /* Create %USERPROFILE%/bin */
    homeDir = getEnvironmentVariable(L"USERPROFILE");
    binDir = buildPath(homeDir, L"bin");
    if (! fileExists(binDir)) {
        wprintf(L"Creating install directory %ls\n", binDir);
        if (CreateDirectory(binDir, NULL) == CREATEDIRECTORY_FAILURE) {
            writeLastError(GetLastError(), L"Failed to create directory", binDir);
            exit(EXIT_FAILURE);
        }
    }
    /* Copy self to bin/du.exe */
    installedProgram = buildPath(binDir, L"du.exe");
    if (endsWith(toLowerCase(programName), L".exe")) {
        exactProgramName = programName;
    } else {
        exactProgramName = concat(programName, L".exe");
    }
    wprintf(L"Installing %ls.\n", installedProgram);
    if (CopyFile(exactProgramName, installedProgram, FALSE) == COPYFILE_FAILURE) {
        writeLastError(GetLastError(), L"Failed to copy self to", installedProgram);
        exit(EXIT_FAILURE);
    }
    /* Update user PATH in registry */
    addElementToRegistryUserPath(binDir);
}

static void du(int argc, const wchar_t *argv[])
{
    List *fileArgs;
    List *node;
    wchar_t *argument;

    fileArgs = setSwitches(argc, argv);
    printStringList(fileArgs);
    if (getListSize(fileArgs) > 0) {
        for (node = fileArgs; !isListEmpty(node); node = skipListItem(node)) {
            argument = removeListItem(&fileArgs);
            calcDiskUsage(argument, true);
        }
    } else {
        argument = getAbsolutePath(DEFAULT_PATH);
        calcDiskUsage(argument, true);
    }
}

void printFileSize(wchar_t *path, int64_t size) {
    double hrSize;
    if (humanReadable) {
        if (size >= GIBIBYTE) {
            hrSize = ((double) size) / ((double) GIBIBYTE);
            _tprintf(_T("%2.1fG\t%ls\n"), hrSize, path);
        } else if (size >= MEBIBYTE) {
            hrSize = ((double) size) / ((double) MEBIBYTE);
            _tprintf(_T("%2.1fM\t%ls\n"), hrSize, path);
        } else if (size >= KIBIBYTE) {
            hrSize = ((double) size) / ((double) KIBIBYTE);
            _tprintf(_T("%2.1fK\t%ls\n"), hrSize, path);
        } else {
            _tprintf(_T("%-2I64d\t%ls\n"), size, path);
        }
    } else {
        if (!displayBytes) {
            if (size > 0) {
                size = size / ((unsigned long) (1024.0 + 0.5)); /* Convert to KB and round */
                if (size == 0) {
                    size = 1; /* Don't allow zero to display if there are bytes in the file */
                }
            }
        }
        wprintf(L"%-7I64d %ls\n", size, path);
    }
    fflush(stdout);
}

int64_t calcDiskUsage(wchar_t *path, bool isTopLevel) {
    int64_t size = 0;
    List* entries;
    List* entry;
    if (isFile(path)) {
        size = getFileSize(path);
        if (displayRegularFilesAlso || isTopLevel) {
            printFileSize(path, size);
        }
    } else {
        entries = listFiles(path);
        for (entry = entries; !isListEmpty(entry); entry = skipListItem(entry)) {
            size += calcDiskUsage((wchar_t*) getListItem(entry), false);
        }
        if (!summarize || isTopLevel) {
            printFileSize(path, size);
        }
    }
    return size;
}
