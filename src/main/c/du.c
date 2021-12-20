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
#include <gc.h>
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

const wchar_t *programName;

int wmain(int argc, const wchar_t *argv[]) {
	List *fileArgs;
	List *node;
	wchar_t *argument;

	GC_INIT();
	programName = argv[0];
	fileArgs = setSwitches(argc, argv);
	if (getListSize(fileArgs) > 0) {
		for (node = fileArgs; !isListEmpty(node); skipListItem(node)) {
			argument = removeListItem(&fileArgs);
			calcDiskUsage(argument, true);
		}
	}
	else {
		argument = getAbsolutePath(DEFAULT_PATH);
		calcDiskUsage(argument, true);
	}
	return EXIT_SUCCESS;
}

void printFileSize(wchar_t *path, unsigned long size) {
	double hrSize;
	if (humanReadable) {
		if (size >= GIBIBYTE) {
			hrSize = ((double) size) / ((double) GIBIBYTE);
			_tprintf(_T("%2.1fG\t%s\n"), hrSize, getSimpleName(path));
		}
		else if (size >= MEBIBYTE) {
			hrSize = ((double) size) / ((double) MEBIBYTE);
			_tprintf(_T("%2.1fM\t%s\n"), hrSize, getSimpleName(path));
		}
		else if (size >= KIBIBYTE) {
			hrSize = ((double) size) / ((double) KIBIBYTE);
			_tprintf(_T("%2.1fK\t%s\n"), hrSize, getSimpleName(path));
		}
		else {
			_tprintf(_T("%-2lu\t%s\n"), size, getSimpleName(path));
		}
	}
	else {
		if (!displayBytes) {
			if (size > 0) {
				size = size / ((unsigned long) (1024.0 + 0.5)); /* Convert to KB and round */
				if (size == 0) {
					size = 1; /* Don't allow zero to display if there are bytes in the file */
				}
			}
		}
		_tprintf(_T("%-7lu %s\n"), size, getSimpleName(path));
	}
}

unsigned long calcDiskUsage(wchar_t *path, bool isTopLevel)
{
    unsigned long size = 0;
    List *entries;
    List *entry;

	if (isFile(path)) {
		size = getFileSize(path);
		if (displayRegularFilesAlso || isTopLevel) {
			printFileSize(path, size);
		}
	}
	else {
		entries = listFiles(path);
		for (entry = entries; !isListEmpty(entry); skipListItem(entry)) {
			size += calcDiskUsage((wchar_t*) getListItem(entry), false);
		}
		if (!summarize || isTopLevel) {
			printFileSize(path, size);
		}
	}
	return size;
}
