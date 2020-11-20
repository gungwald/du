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

static void getSizeOfArgument(const TCHAR *argument);
static void usage();
static void version();
static int removeElement(int argc, TCHAR *argv[], int elementNumber);
static List *setSwitches(int argc, TCHAR *argv[]);
static void printFileSize(Path *path, unsigned long size);
static unsigned long getSize(Path *path, bool isTopLevel);
static unsigned long getSizeOfDirectory(Path *path, bool isTopLevel);
static unsigned long getSizeOfRegularFile(Path *path, bool isTopLevel);
static void getSizesOfMatchingFiles(Path *path, bool isTopLevel);

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
bool humanReadable = false;
TCHAR *programName;

int _tmain(int argc, TCHAR *argv[]) {
	List *nonSwitchArguments;

	TRACE_ENTER(_T("wmain"), _T("argv[1]"), argv[1]);
	programName = argv[0];
	nonSwitchArguments = setSwitches(argc, argv);
	if (listGetSize(nonSwitchArguments) > 0) {
		while (listHasMoreElements(nonSwitchArguments)) {
			getSizeOfArgument(listGetData(nonSwitchArguments));
			listAdvance(nonSwitchArguments);
		}
	}
	else {
		getSizeOfArgument(DEFAULT_PATH);
	}
	delete_List(nonSwitchArguments);
	TRACE_RETURN_INT(_T("wmain"), EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

void getSizeOfArgument(const TCHAR *argument)
{
	Path *path;

	TRACE_ENTER(_T("getSizeOfArgument"), _T("argument"), argument);
	path = new_Path(argument);
	if (isGlob(argument)) { /* Will be true if compiled with Visual C++, but not with GCC. */
		getSizesOfMatchingFiles(path, true);
	}
	else {
		getSize(path, true);
	}
	delete_Path(path);
	TRACE_RETURN(_T("getSizeOfArgument"), _T("void"));
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

int removeElement(int argc, TCHAR *argv[], int elementNumber) {
	int i;

	for (i = elementNumber; i < argc - 1; i++) {
		argv[i] = argv[i + 1];
	}
	return i;
}

List *setSwitches(int argc, TCHAR *argv[]) {
	int i;
	TCHAR *argument;
	List *remainingArguments;

	remainingArguments = new_List();
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
			listAdd(remainingArguments, argument);
		}
	}
	if (displayRegularFilesAlso && summarize) {
		_ftprintf(stderr, _T("%s: ERROR with arguments: cannot both summarize and show all entries\n"), programName);
		exit(EXIT_FAILURE);
	}
	return remainingArguments;
}

void printFileSize(Path *path, unsigned long size) {
	if (! displayBytes) {
		if (size > 0) {
			size = size / ((unsigned long) (1024.0 + 0.5)); /* Convert to KB and round */
			if (size == 0) {
				size = 1; /* Don't allow zero to display if there are bytes in the file */
			}
		}
	}
	_tprintf(_T("%-7lu %s\n"), size, pathGetOriginal(path));
}

void getSizesOfMatchingFiles(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	bool moreMatchesForThisArgument = true;
	DWORD lastError;
	const TCHAR *entryBasename;
	Path *searchDirectory;
	Path *directoryEntry;

	TRACE_ENTER_CALLBACK(_T("displaySizesOfMatchingFiles"), _T("path"), pathDump, path);

	findHandle = FindFirstFile(pathGetAbsoluteRaw(path), &fileProperties);
	if (findHandle == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for search pattern"), pathGetAbsolute(path));
	}
	else {
		searchDirectory = pathDirName(path);
		while (moreMatchesForThisArgument) {
			entryBasename = fileProperties.cFileName;
			if (_tcscmp(entryBasename, _T(".")) != 0 && _tcscmp(entryBasename, _T("..")) != 0) {

				directoryEntry = pathAppend(searchDirectory, entryBasename);
				getSize(directoryEntry, true);
				free(directoryEntry);
			}
			if (!FindNextFile(findHandle, &fileProperties)) {
				lastError = GetLastError();
				if (lastError == ERROR_NO_MORE_FILES) {
					moreMatchesForThisArgument = false;
				}
				else {
					writeLastError(lastError, _T("failed to get next file matching pattern"), pathGetAbsolute(path));
				}
			}
		}
		free(searchDirectory);
		FindClose(findHandle); /* Only close it if it got opened successfully */
	}
	TRACE_RETURN(_T("displaySizesOfMatchingFiles"), _T("void"));
}

unsigned long getSizeOfDirectory(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	bool moreDirectoryEntries = true;
	Path *searchPattern;
	const TCHAR *entryBasename;
	Path *dirEntry;
	DWORD lastError;
	unsigned long size = 0;

	TRACE_ENTER_CALLBACK(_T("getSizeOfDirectory"), _T("path"), pathDump, path);

	searchPattern = pathAppend(path, _T("*"));
	if (searchPattern != NULL) {
		findHandle = FindFirstFile(pathGetAbsoluteRaw(searchPattern), &fileProperties);
		if (findHandle == INVALID_HANDLE_VALUE) {
			writeLastError(GetLastError(), _T("failed to get handle for file search pattern"), pathGetAbsolute(searchPattern));
		}
		else {
			while (moreDirectoryEntries) {
				entryBasename = fileProperties.cFileName;
				if (_tcscmp(entryBasename, _T(".")) != 0 && _tcscmp(entryBasename, _T("..")) != 0) {
					if ((dirEntry = pathAppend(path, entryBasename)) != NULL) {
						size += getSize(dirEntry, false); /* RECURSION */
						free(dirEntry);
					}
				}
				if (!FindNextFile(findHandle, &fileProperties)) {
					if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
						moreDirectoryEntries = false;
					}
					else {
						writeLastError(lastError, _T("failed to get next file search results"), pathGetAbsolute(searchPattern));
					}
				}
			}
			FindClose(findHandle); /* Only close it if it got opened successfully */
			if (!summarize || isTopLevel) {
				printFileSize(path, size);
			}
		}
		free(searchPattern);
	}
	TRACE_RETURN_ULONG(_T("getSizeOfDirectory"), size);
	return size;
}

unsigned long getSizeOfRegularFile(Path *path, bool isTopLevel) {
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	unsigned long size = 0;
	unsigned long multiplier;
	unsigned long maxDWORD;

	TRACE_ENTER_CALLBACK(_T("getSizeOfRegularFile"), _T("path"), pathDump, path);

	findHandle = FindFirstFile(pathGetAbsoluteRaw(path), &fileProperties);
	if (findHandle == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for file"), pathGetAbsolute(path));
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
	TRACE_RETURN_ULONG(_T("getSizeOfRegularFile"), size);
	return size;
}

unsigned long getSize(Path *path, bool isTopLevel) {
	DWORD fileAttributes;
	unsigned long size = 0;

	TRACE_ENTER_CALLBACK(_T("getSize"), _T("path"), pathDump, path);

	fileAttributes = GetFileAttributes(pathGetAbsoluteRaw(path));
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		writeLastError(GetLastError(), _T("failed to get attributes for file"), pathGetAbsolute(path));
	}
	else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		size = getSizeOfDirectory(path, isTopLevel); /* RECURSION */
	}
	else {
		size = getSizeOfRegularFile(path, isTopLevel);
	}
	TRACE_RETURN_ULONG(_T("getSize"), size);
	return size;
}
