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
#include <tchar.h>
#include "path.h"
#include "build-number.h"
#include "version.h"
#include "string-utils.h"
#include "error-handling.h"
#include "trace.h"
#include "list.h"

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

#define DEFAULT_PATH _T(".")
#define FIND_ALL_PATTERN _T("\\*")

#define KIBIBYTE 400
#define MEBIBYTE 100000
#define GIBIBYTE 40000000

static void usage();
static void version();
static List *setSwitches(int argc, TCHAR *argv[]);
static void printFileSize(Path *path, unsigned long size);
static unsigned long calcDiskUsageOfArgument(const TCHAR *argument);
static unsigned long calcDiskUsageOfFile(Path *path, bool isTopLevel);
static unsigned long calcDiskUsageOfDirectory(Path *path, bool isTopLevel);
static unsigned long calcDiskUsageOfRegularFile(Path *path, bool isTopLevel);
static unsigned long calcDiskUsageOfFilesMatchingPattern(Path *path, bool isTopLevel);

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
bool humanReadable = false;
TCHAR *programName;

int _tmain(int argc, TCHAR *argv[]) {
	List *nonSwitchArguments;

	TRACE_ENTER(__func__, _T("argv[1]"), argv[1]);
	programName = argv[0];
	nonSwitchArguments = setSwitches(argc, argv);
	if (list_GetSize(nonSwitchArguments) > 0) {
		while (list_HasMoreElements(nonSwitchArguments)) {
			calcDiskUsageOfArgument(list_GetData(nonSwitchArguments));
			list_Advance(nonSwitchArguments);
		}
	}
	else {
		calcDiskUsageOfArgument(DEFAULT_PATH);
	}
	list_Delete(nonSwitchArguments);
	TRACE_RETURN_INT(__func__, EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

unsigned long calcDiskUsageOfArgument(const TCHAR *argument)
{
	Path *path;
	size_t diskUsage;

	TRACE_ENTER(__func__, _T("argument"), argument);
	path = path_Init(argument);
	if (isGlob(argument)) { /* Will be true if compiled with Visual C++, but not with GCC. */
		diskUsage = calcDiskUsageOfFilesMatchingPattern(path, true);
	}
	else {
		diskUsage = calcDiskUsageOfFile(path, true);
	}
	path_Delete(path);
	TRACE_RETURN_ULONG(__func__, diskUsage);
	return diskUsage;
}

void usage() {
	_putts(_T("Usage: du [OPTION]... [FILE]..."));
	_putts(_T("Summarize disk usage of each FILE, recursively for directories."));
	_putts(_T("File and directory sizes are written in kilobytes."));
	_putts(_T("1 kilobyte = 1024 bytes"));
	_putts(_T(""));
	_putts(_T("  /a, -a, --all            write counts for all files, not just directories"));
	_putts(_T("  /b, -b, --bytes          print size in bytes"));
	_putts(_T("  /h, -h, --human-readable print sizes in human readable format (e.g., 1K 234M 2G)"));
	_putts(_T("  /s, -s, --summarize      display only a total for each argument"));
	_putts(_T("  /?, -?, --help           display this help and exit"));
	_putts(_T("  /v, -v, --version        output version information and exit"));
	_putts(_T(""));
	_putts(_T("Example: du -s *"));
	_putts(_T(""));
	_putts(_T("Report bugs at https://github.com/gungwald/du"));
}

void version() {
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

List *setSwitches(int argc, TCHAR *argv[]) {
	int i;
	TCHAR *argument;
	List *remainingArguments;

	remainingArguments = list_Init();
	for (i = 1; i < argc; i++) {
		argument = argv[i];
		if (_tcscmp(argument, _T("/?")) == 0 || _tcscmp(argument, _T("-?")) == 0 || _tcscmp(argument, _T("--help")) == 0) {
			usage();
			exit(EXIT_SUCCESS);	/* The Linux man page says to exit after printing help. */
		}
		else if (_tcscmp(argument, _T("/v")) == 0 || _tcscmp(argument, _T("-v")) == 0 || _tcscmp(argument, _T("--version")) == 0) {
			version();
			exit(EXIT_SUCCESS);	/* The Linux man page says to exit after printing the version. */
		}
		else if (_tcscmp(argument, _T("/a")) == 0 || _tcscmp(argument, _T("-a")) == 0 || _tcscmp(argument, _T("--all")) == 0) {
			displayRegularFilesAlso = true;
		}
		else if (_tcscmp(argument, _T("/b")) == 0 || _tcscmp(argument, _T("-b")) == 0 || _tcscmp(argument, _T("--bytes")) == 0) {
			displayBytes = true;
		}
		else if (_tcscmp(argument, _T("/s")) == 0 || _tcscmp(argument, _T("-s")) == 0 || _tcscmp(argument, _T("--summarize")) == 0) {
			summarize = true;
		}
		else if (_tcscmp(argument, _T("/h")) == 0 || _tcscmp(argument, _T("-h")) == 0 || _tcscmp(argument, _T("--human-readable")) == 0) {
			humanReadable = true;
		}
		else {
			list_Append(remainingArguments, argument);
		}
	}
	if (displayRegularFilesAlso && summarize) {
		_ftprintf(stderr, _T("%s: ERROR with arguments: cannot both summarize and show all entries\n"), programName);
		exit(EXIT_FAILURE);
	}
	return remainingArguments;
}

void printFileSize(Path *path, unsigned long size) {
	double hrSize;
	if (humanReadable) {
		if (size >= GIBIBYTE) {
			hrSize = ((double) size) / ((double) GIBIBYTE);
			_tprintf(_T("%.1fG    %s\n"), hrSize, path_GetOriginal(path));
		}
		else if (size >= MEBIBYTE) {
			hrSize = ((double) size) / ((double) MEBIBYTE);
			_tprintf(_T("%.1fM    %s\n"), hrSize, path_GetOriginal(path));
		}
		else if (size >= KIBIBYTE) {
			hrSize = ((double) size) / ((double) KIBIBYTE);
			_tprintf(_T("%.1fK    %s\n"), hrSize, path_GetOriginal(path));
		}
		else {
			_tprintf(_T("%-7lu %s\n"), size, path_GetOriginal(path));
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
		_tprintf(_T("%-7lu %s\n"), size, path_GetOriginal(path));
	}
}

unsigned long calcDiskUsageOfFilesMatchingPattern(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	bool moreMatchesForThisArgument = true;
	DWORD lastError;
	const TCHAR *entryBasename;
	Path *searchDirectory;
	Path *directoryEntry;
	unsigned long totalSize = 0;

	TRACE_ENTER_CALLBACK(__func__, _T("path"), path_Dump, path);

	findHandle = FindFirstFile(path_GetAbsoluteRaw(path), &fileProperties);
	if (findHandle == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for search pattern"), path_GetAbsolute(path));
	}
	else {
		searchDirectory = path_GetParentDirectory(path);
		while (moreMatchesForThisArgument) {
			entryBasename = fileProperties.cFileName;
			if (_tcscmp(entryBasename, _T(".")) != 0 && _tcscmp(entryBasename, _T("..")) != 0) {
				directoryEntry = path_Append(searchDirectory, entryBasename);
				totalSize += calcDiskUsageOfFile(directoryEntry, true);
				path_Delete(directoryEntry);
			}
			if (!FindNextFile(findHandle, &fileProperties)) {
				lastError = GetLastError();
				if (lastError == ERROR_NO_MORE_FILES) {
					moreMatchesForThisArgument = false;
				}
				else {
					writeLastError(lastError, _T("failed to get next file matching pattern"), path_GetAbsolute(path));
				}
			}
		}
		path_Delete(searchDirectory);
		FindClose(findHandle); /* Only close it if it got opened successfully */
	}
	TRACE_RETURN_ULONG(__func__, totalSize);
	return totalSize;
}

unsigned long calcDiskUsageOfDirectory(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	bool moreDirectoryEntries = true;
	Path *searchPattern;
	const TCHAR *entryBasename;
	Path *dirEntry;
	DWORD lastError;
	unsigned long size = 0;

	TRACE_ENTER_CALLBACK(__func__, _T("path"), path_Dump, path);

	searchPattern = path_Append(path, _T("*"));
	if (searchPattern != NULL) {
		findHandle = FindFirstFile(path_GetAbsoluteRaw(searchPattern), &fileProperties);
		if (findHandle == INVALID_HANDLE_VALUE) {
			writeLastError(GetLastError(), _T("failed to get handle for file search pattern"), path_GetAbsolute(searchPattern));
		}
		else {
			while (moreDirectoryEntries) {
				entryBasename = fileProperties.cFileName;
				if (_tcscmp(entryBasename, _T(".")) != 0 && _tcscmp(entryBasename, _T("..")) != 0) {
					if ((dirEntry = path_Append(path, entryBasename)) != NULL) {
						size += calcDiskUsageOfFile(dirEntry, false); /* RECURSION */
						path_Delete(dirEntry);
					}
				}
				if (!FindNextFile(findHandle, &fileProperties)) {
					if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
						moreDirectoryEntries = false;
					}
					else {
						writeLastError(lastError, _T("failed to get next file search results"), path_GetAbsolute(searchPattern));
					}
				}
			}
			FindClose(findHandle); /* Only close it if it got opened successfully */
			if (!summarize || isTopLevel) {
				printFileSize(path, size);
			}
		}
		path_Delete(searchPattern);
	}
	TRACE_RETURN_ULONG(__func__, size);
	return size;
}

unsigned long calcDiskUsageOfRegularFile(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	unsigned long size = 0;
	unsigned long multiplier;
	unsigned long maxDWORD;

	TRACE_ENTER_CALLBACK(__func__, _T("path"), path_Dump, path);

	findHandle = FindFirstFile(path_GetAbsoluteRaw(path), &fileProperties);
	if (findHandle == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for file"), path_GetAbsolute(path));
	}
	else {
		maxDWORD = (unsigned long) MAXDWORD; /* Avoid Visual C++ 4.0 warning */
		multiplier = maxDWORD + 1UL;
		size = fileProperties.nFileSizeHigh * multiplier + fileProperties.nFileSizeLow;
		FindClose(findHandle);
		if (displayRegularFilesAlso || isTopLevel) {
			printFileSize(path, size);
		}
	}
	TRACE_RETURN_ULONG(__func__, size);
	return size;
}

unsigned long calcDiskUsageOfFile(Path *path, bool isTopLevel) {
	DWORD fileAttributes;
	unsigned long size = 0;

	TRACE_ENTER_CALLBACK(__func__, _T("path"), path_Dump, path);

	fileAttributes = GetFileAttributes(path_GetAbsoluteRaw(path));
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		writeLastError(GetLastError(), _T("failed to get attributes for file"), path_GetAbsolute(path));
	}
	else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		size = calcDiskUsageOfDirectory(path, isTopLevel); /* RECURSION */
	}
	else {
		size = calcDiskUsageOfRegularFile(path, isTopLevel);
	}
	TRACE_RETURN_ULONG(__func__, size);
	return size;
}
