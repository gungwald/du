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

/* TODO - Fix argument handling */
/* TODO - Implement human readable option */

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
#include "filename.h"
#include "string.h"
#include "error.h"
#include "trace.h"
#include "list.h"
#include "args.h"
#include "help.h"

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

#define DEFAULT_PATH _T(".")
#define FIND_ALL_PATTERN _T("\\*")

#define KIBIBYTE 0x400
#define MEBIBYTE 0x100000
#define GIBIBYTE 0x40000000

static void printFileSize(wchar_t *path, unsigned long size);
static unsigned long calcDiskUsage(wchar_t *path, bool isTopLevel);

wchar_t *programName;

int wmain(int argc, wchar_t *argv[])
{
    List *fileArgs;
    List *node;
    wchar_t *argument;

    programName = argv[0];
    fileArgs = setSwitches(argc, argv);
    if (getListSize(fileArgs) > 0) {
        for (node = fileArgs; ! isListEmpty(node); skipListNode(node)) {
            argument = removeListNode(fileArgs);
            calcDiskUsage(argument, true);
            free(argument);
        }
    } else {
        argument = getAbsolutePath(DEFAULT_PATH);
        calcDiskUsage(argument, true);
        free(argument);
    }
    freeList(fileArgs);
    return EXIT_SUCCESS;
}

void printFileSize(wchar_t *f, unsigned long size)
{
    double hrSize;
    if (humanReadable) {
        if (size >= GIBIBYTE) {
            hrSize = ((double) size) / ((double) GIBIBYTE);
            _tprintf(_T("%2.1fG\t%s\n"), hrSize, getSimpleName(f));
        } else if (size >= MEBIBYTE) {
            hrSize = ((double) size) / ((double) MEBIBYTE);
            _tprintf(_T("%2.1fM\t%s\n"), hrSize, getSimpleName(f));
        } else if (size >= KIBIBYTE) {
            hrSize = ((double) size) / ((double) KIBIBYTE);
            _tprintf(_T("%2.1fK\t%s\n"), hrSize, getSimpleName(f));
        } else {
            _tprintf(_T("%-2lu\t%s\n"), size, getSimpleName(f));
        }
    } else {
        if (!displayBytes) {
            if (size > 0) {
                /* Convert to KB and round */
                size = size / ((unsigned long) (1024.0 + 0.5));
                if (size == 0) {
                    /* Don't allow zero to display if there are bytes in the file */
                    size = 1;
                }
            }
        }
        _tprintf(_T("%-7lu %s\n"), size, getSimpleName(f));
    }
}

unsigned long calcDiskUsage(wchar_t *f, bool isTopLevel)
{
    unsigned long size = 0;
    List *entries;

    if (isFile(f)) {
        size = getFileSize(f);
        if (displayRegularFilesAlso || isTopLevel) {
            printFileSize(f, size);
        }
    } else {
        for (entries = listDirContents(f); !isListEmpty(entries); skipListNode(entries)) {
            size += calcDiskUsage((wchar_t *) getListNodeData(entries), false);
        }
        freeList(entries);
        if (!summarize || isTopLevel) {
            printFileSize(f, size);
        }
    }
    return size;
}
