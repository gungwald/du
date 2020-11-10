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

/**
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
#include <lmerr.h>

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

#define ERROR_TEXT_CAPACITY 128

bool isGlob(const _TCHAR *fileName);
void displayErrorText(DWORD errorCode);
void usage();
void version();
int removeElement(int argc, _TCHAR* argv[], int elementNumber);
int setSwitches(int argc, _TCHAR* argv[]);
void printFileSize(const _TCHAR *fileName, unsigned long size);
unsigned long getSize(const _TCHAR *fileName, bool isTopLevel);
unsigned long getSizeOfDirectory(const _TCHAR* name, bool isTopLevel);
unsigned long getSizeOfRegularFile(const _TCHAR* name, bool isTopLevel);
void writeError(errno_t errorCode, const _TCHAR* message, const _TCHAR* object);
void writeError2(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR *object2);
void writeError3(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR *object2, const _TCHAR *object3);
void writeLastError(DWORD lastError, const _TCHAR* message, const _TCHAR* object);
_TCHAR* concat(const _TCHAR* left, const _TCHAR* right);
_TCHAR* concat3(const _TCHAR* first, const _TCHAR* second, const _TCHAR *third);

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
_TCHAR* programName;

int _tmain(int argc, _TCHAR *argv[])
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool doneWithThisArgument = false;
    DWORD lastError;
    int i;
    LPCTSTR argument;
    LPCTSTR fileName;
    int nonSwitchArgumentCount;

    programName = argv[0];
    nonSwitchArgumentCount = setSwitches(argc, argv);
    if (nonSwitchArgumentCount > 1) {
        for (i = 1; i < nonSwitchArgumentCount; i++) {
            argument = argv[i];
            if (isGlob(argument)) {  /* Will be true if compiled with Visual C++, but not with GCC. */
                if ((findHandle = FindFirstFile(argument, &fileProperties)) == INVALID_HANDLE_VALUE) {
                    writeLastError(GetLastError(), _TEXT("Failed to get handle for search pattern"), argument);
                }
                else {
                    doneWithThisArgument = false;
                    while (!doneWithThisArgument) {
                        fileName = fileProperties.cFileName;
                        if (_tcscmp(fileName, _TEXT(".")) != 0 && _tcscmp(fileName, _TEXT("..")) != 0) {
                            getSize(fileName, true);
                        }
                        if (!FindNextFile(findHandle, &fileProperties)) {
                            lastError = GetLastError();
                            if (lastError == ERROR_NO_MORE_FILES) {
                                doneWithThisArgument = true;
                            }
                            else {
                                writeLastError(lastError, _TEXT("Failed to get next file matching pattern"), argument);
                            }
                        }
                    }
                    FindClose(findHandle);  /* Only close it if it got opened successfully */
                }
            }
            else {
                getSize(argument, true);
            }
        }
    }
    else {
        getSize(_TEXT("."), true);
    }
    return EXIT_SUCCESS;
}

bool isGlob(const _TCHAR *argument)
{
    return _tcschr(argument, _TEXT('*')) != NULL || _tcschr(argument, _TEXT('?')) != NULL;
}

/* This function was taken from Microsoft's Knowledge Base Article 149409
   and modified to fix the formatting. */
void displayErrorText(DWORD errorCode)
{
    HMODULE moduleHandle = NULL; /* default to system source */
    _TCHAR *message;
    DWORD bufferLength;
    DWORD numberOfBytesWritten;

    /* If errorCode is in the network range, load the message source */
    if (errorCode >= NERR_BASE && errorCode <= MAX_NERR) {
        moduleHandle = LoadLibraryEx(_TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (moduleHandle == NULL) {
            /* Can't call writeLastError because that could cause an infinite recursive failure loop. */
            _ftprintf(stderr, _TEXT("Failed to load library netmsg.dll: error number %d\n"), GetLastError());
        }
    }

    /* Call FormatMessage() to allow for message text to be acquired
       from the system or the supplied module handle */
    bufferLength = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM | /* always consider system table */
        ((moduleHandle != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
        moduleHandle, /* Module to get message from (NULL == system) */
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
        (LPTSTR)&message,
        0,
       NULL);

    if (bufferLength) {
        /* Output message string on stderr */
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, bufferLength, &numberOfBytesWritten, NULL);
        /* Free the buffer allocated by the system */
        LocalFree(message);
    }
    /* If you loaded a message source, unload it */
    if (moduleHandle != NULL) {
        FreeLibrary(moduleHandle);
    }
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
    _putts(_TEXT("Report bugs to <bill_chatfield@yahoo.com>"));
}

void version()
{
    _putts(_TEXT("du for Windows - Version 1.0"));
    _putts(_TEXT("Copyright(c) 2004 Bill Chatfield <bill_chatfield@yahoo.com>"));
    _putts(_TEXT("Distributed under the GNU General Public License."));
    _putts(_TEXT(""));
    _putts(_TEXT("This du is written to the native WIN32 C API so that"));
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

void printFileSize(const _TCHAR *fileName, unsigned long size)
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

_TCHAR* concat(const _TCHAR* left, const _TCHAR* right)
{
    size_t capacity;
    _TCHAR* result;
    errno_t errorCode;

    capacity = _tcslen(left) + _tcslen(right) + 1;
	if ((result = (_TCHAR *) malloc(capacity * sizeof(_TCHAR))) != NULL) {
        if ((errorCode = _tcscpy_s(result, capacity, left)) == 0) {
            if ((errorCode = _tcscat_s(result, capacity, right)) != 0) {
                writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, right);
                free(result);
                result = NULL;
            }
        }
        else {
            writeError(errorCode, _TEXT("Count will be off because string copy failed"), left);
            free(result);
            result = NULL;
        }
	}
    else {
		writeError2(errno, _TEXT("Memory allocation failed for string concatenation"), left, right);
		exit(EXIT_FAILURE);
    }
    return result;
}

_TCHAR* concat3(const _TCHAR* first, const _TCHAR* second, const _TCHAR *third)
{
    size_t capacity;
    errno_t errorCode;
    _TCHAR* result;

    capacity = _tcslen(first) + _tcslen(second) + _tcslen(third) + 1;
	if ((result = (_TCHAR *) malloc(capacity * sizeof(_TCHAR))) != NULL) {
        if ((errorCode = _tcscpy_s(result, capacity, first)) == 0) {
            if ((errorCode = _tcscat_s(result, capacity, second)) == 0) {
                if ((errorCode = _tcscat_s(result, capacity, third)) != 0) {
                    writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, third);
                    free(result);
                    result = NULL;
                }
            }
            else {
                writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, second);
                free(result);
                result = NULL;
            }
        }
        else {
            writeError(errorCode, _TEXT("Count will be off because string copy failed"), first);
            free(result);
            result = NULL;
        }
	}
    else {
		writeError3(errno, _TEXT("Memory allocation failed for string concatenation"), first, second, third);
		exit(EXIT_FAILURE);
    }
    return result;
}

unsigned long getSizeOfDirectory(const _TCHAR* directoryName, bool isTopLevel)
{
    HANDLE findHandle;
    WIN32_FIND_DATA fileProperties;
    bool moreDirectoryEntries = true;
    _TCHAR *searchPattern;
    const _TCHAR *childFileName;
    _TCHAR *childPath;
    DWORD lastError;
    unsigned long size = 0;

    if ((searchPattern = concat(directoryName, _TEXT("\\*"))) != NULL) {
		if ((findHandle = FindFirstFile(searchPattern, &fileProperties)) == INVALID_HANDLE_VALUE) {
			writeLastError(GetLastError(), _TEXT("Failed to get handle for file listing"), searchPattern);
		}
		else {
			while (moreDirectoryEntries) {
				childFileName = fileProperties.cFileName;
				if (_tcscmp(childFileName, _TEXT(".")) != 0 && _tcscmp(childFileName, _TEXT("..")) != 0) {
                    if ((childPath = concat3(directoryName, _TEXT("\\"), childFileName)) != NULL) {
						size += getSize(childPath, false);  /* RECURSION */
                        free(childPath);
					}
				}
				if (!FindNextFile(findHandle, &fileProperties)) {
					if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
						moreDirectoryEntries = false;
					}
					else {
						writeLastError(lastError, _TEXT("Failed to get next file in directory"), searchPattern);
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
		writeLastError(GetLastError(), _TEXT("Failed to get handle for file"), name);
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

unsigned long getSize(const _TCHAR *fileName, bool isTopLevel)
{
    DWORD fileAttributes;
    unsigned long size = 0;

    if ((fileAttributes = GetFileAttributes(fileName)) == INVALID_FILE_ATTRIBUTES) {
        writeLastError(GetLastError(), _TEXT("Failed to attributes for file"), fileName);
    }
    else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        size = getSizeOfDirectory(fileName, isTopLevel);    /* RECURSION */
    }
    else {
        size = getSizeOfRegularFile(fileName, isTopLevel);
    }
    return size;
}

void writeError(errno_t errorCode, const _TCHAR* message, const _TCHAR* object)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

	_tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
	_ftprintf(stderr, _TEXT("%s: %s: \"%s\": %s\n"), programName, message, object, errorText);
}

void writeError2(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR *object2)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

	_tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
	_ftprintf(stderr, _TEXT("%s: %s: \"%s\" and \"%s\": %s\n"), programName, message, object1, object2, errorText);
}

void writeError3(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR *object2, const _TCHAR *object3)
{
    _TCHAR errorText[ERROR_TEXT_CAPACITY];

	_tcserror_s(errorText, ERROR_TEXT_CAPACITY, errorCode);
	_ftprintf(stderr, _TEXT("%s: %s: \"%s\", \"%s\" and \"%s\": %s\n"), programName, message, object1, object2, object3, errorText);
}

void writeLastError(DWORD lastError, const _TCHAR* message, const _TCHAR* object)
{
	_ftprintf(stderr, _TEXT("%s: %s: \"%s\": "), programName, message, object);
	displayErrorText(lastError);
	_fputts(_TEXT(""), stderr);  /* Write end of line */
}
