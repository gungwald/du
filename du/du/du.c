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
#include <windows.h>
#include <tchar.h>
#include <lmerr.h>

/* Visual C++ 4.0 does not define this. */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

int isGlob(LPCTSTR lpFileName);
void displayErrorText(DWORD errorCode);
void usage();
void version();
int removeElement(int argc, TCHAR* argv[], int elementNumber);
int setSwitches(int argc, TCHAR* argv[]);
void printFileSize(LPCTSTR fileName, unsigned long size);
unsigned long getSize(LPCTSTR fileName, int isTopLevel);

int displayRegularFilesAlso = 0;
int displayBytes = 0;
int summarize = 0;

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

int main(int argc, TCHAR* argv[])
{
    HANDLE hFind;
    WIN32_FIND_DATA findFileData;
    int doneWithThisArg = 0;
    DWORD lastError;
    int i;
    LPCTSTR argument;
    LPCTSTR fileName;

    argc = setSwitches(argc, argv);
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            argument = argv[i];
            if (isGlob(argument)) {  /* Will be true if compiled with Visual C++, but not with GCC. */
                hFind = FindFirstFile(argument, &findFileData);
                if (hFind == INVALID_HANDLE_VALUE) {
                    lastError = GetLastError();
                    _ftprintf(stderr, _TEXT("du: ERROR getting handle for search pattern ``%s'': "), argument);
                    displayErrorText(lastError);
                    _fputts(_TEXT(""), stderr);  /* Write end of line */
                }
                else {
                    doneWithThisArg = 0;
                    while (!doneWithThisArg) {
                        fileName = findFileData.cFileName;
                        if (_tcscmp(fileName, _TEXT(".")) != 0 && _tcscmp(fileName, _TEXT("..")) != 0) {
                            getSize(fileName, 1);
                        }
                        if (!FindNextFile(hFind, &findFileData)) {
                            lastError = GetLastError();
                            if (lastError == ERROR_NO_MORE_FILES) {
                                doneWithThisArg = 1;
                            }
                            else {
                                _ftprintf(stderr, _TEXT("du: ERROR getting next file in pattern ``%s'': "), argument);
                                displayErrorText(lastError);
                                _fputts(_TEXT(""), stderr);  /* Write end of line */
                            }
                        }
                    }
                    FindClose(hFind);  /* Only close it if it got opened successfully */
                }
            }
            else {
                getSize(argument, 1);
            }
        }
    }
    else {
        getSize(_TEXT("."), 1);
    }
    /* system("PAUSE"); */
    return 0;
}

int isGlob(LPCTSTR argument)
{
    return _tcschr(argument, _TEXT('*')) != NULL || _tcschr(argument, _TEXT('?')) != NULL;
}

/* This function was taken from Microsoft's Knowledge Base Article 149409
   and modified to fix the formatting. */
void displayErrorText(DWORD errorCode)
{
    HMODULE moduleHandle = NULL; /* default to system source */
    LPTSTR message;
    DWORD bufferLength;
    DWORD numberOfBytesWritten;

    /* If dwLastError is in the network range, load the message source */
    if (errorCode >= NERR_BASE && errorCode <= MAX_NERR) {
        moduleHandle = LoadLibraryEx(_TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
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
        if (_tcscmp(argv[i], _TEXT("/?")) == 0 || _tcscmp(argv[i], _TEXT("-?")) == 0 || _tcscmp(argv[i], _TEXT("--help")) == 0) {
            usage();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _TEXT("--version")) == 0) {
            version();
            exit(EXIT_SUCCESS);
        }
        else if (_tcscmp(argv[i], _TEXT("/a")) == 0 || _tcscmp(argv[i], _TEXT("-a")) == 0) {
            displayRegularFilesAlso = 1;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _TEXT("/b")) == 0 || _tcscmp(argv[i], _TEXT("-b")) == 0) {
            displayBytes = 1;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else if (_tcscmp(argv[i], _TEXT("/s")) == 0 || _tcscmp(argv[i], _TEXT("-s")) == 0) {
            summarize = 1;
            newArgumentCount = removeElement(newArgumentCount, argv, i);
        }
        else {
            i++;
        }
    }
    if (displayRegularFilesAlso && summarize) {
        _ftprintf(stderr, _TEXT("du: ERROR with arguments: cannot both summarize and show all entries\n"));
        exit(EXIT_FAILURE);
    }
    return newArgumentCount;
}

void printFileSize(LPCTSTR fileName, unsigned long size)
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

unsigned long getSize(LPCTSTR fileName, int isTopLevel)
{
    unsigned long size = 0;
    unsigned long multiplier;
    unsigned long maxdword;
    DWORD fileAttr;
    HANDLE hFind;
    WIN32_FIND_DATA findFileData;
    int done = 0;
    DWORD lastError;
    errno_t errorCode;
    TCHAR searchPattern[MAX_PATH + 1];
    LPCTSTR childFileName;
    TCHAR childPath[MAX_PATH + 1];

    fileAttr = GetFileAttributes(fileName);
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {  /* An error occured */
        lastError = GetLastError();
        _ftprintf(stderr, _TEXT("du: ERROR getting attributes for file ``%s'': "), fileName);
        displayErrorText(lastError);
        _fputts(_TEXT(""), stderr);  /* Write end of line */
    }
    else if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {  /* It is a directory */
        errorCode = _tcscpy_s(searchPattern, MAX_PATH + 1, fileName);
        if (errorCode != 0) {
            _ftprintf(stderr, _TEXT("du: ERROR count will be off because copying failed for directory name ``%s'': %s\n"), fileName, _tcserror(errorCode));
            return 0;
        }
        errorCode = _tcscat_s(searchPattern, MAX_PATH+1, _TEXT("\\*"));
        if (errorCode != 0) {
            _ftprintf(stderr, _TEXT("du: ERROR count will be off because copying failed for directory search pattern ``%s'': %s\n"), fileName, _tcserror(errorCode));
            return 0;
        }
        hFind = FindFirstFile(searchPattern, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            lastError = GetLastError();
            _ftprintf(stderr, _TEXT("du: ERROR getting handle for file listing ``%s'': "), searchPattern);
            displayErrorText(lastError);
            _fputts(_TEXT(""), stderr);  /* Write end of line */
        }
        else {
            while (!done) {
                childFileName = findFileData.cFileName;
                if (_tcscmp(childFileName, _TEXT(".")) != 0 && _tcscmp(childFileName, _TEXT("..")) != 0) {
                    errorCode = _tcscpy_s(childPath, MAX_PATH+1, fileName);
					if (errorCode != 0) {
						_ftprintf(stderr, _TEXT("du: ERROR count will be off because copying failed for file name ``%s'': %s\n"), fileName, _tcserror(errorCode));
                        continue;
					}
                    errorCode = _tcscat_s(childPath, MAX_PATH+1, _TEXT("\\"));
					if (errorCode != 0) {
						_ftprintf(stderr, _TEXT("du: ERROR count will be off because concat failed for file name ``%s'': %s\n"), childPath, _tcserror(errorCode));
                        continue;
					}
                    errorCode = _tcscat_s(childPath, MAX_PATH+1, childFileName);
					if (errorCode != 0) {
						_ftprintf(stderr, _TEXT("du: ERROR count will be off because concat failed for file name ``%s'' and ``%s'': %s\n"), childPath, childFileName, _tcserror(errorCode));
                        continue;
					}
                    size += getSize(childPath, 0);
                }
                if (!FindNextFile(hFind, &findFileData)) {
                    lastError = GetLastError();
                    if (lastError == ERROR_NO_MORE_FILES) {
                        done = 1;
                    }
                    else {
                        _ftprintf(stderr, _TEXT("du: ERROR getting next file in file listing ``%s'': "), searchPattern);
                        displayErrorText(lastError);
                        _fputts(_TEXT(""), stderr);  /* Write end of line */
                    }
                }
            }
            FindClose(hFind);  /* Only close it if it got opened successfully */
            if (!summarize || isTopLevel) {
                printFileSize(fileName, size);
            }
        }
    }
    else {  /* It is a regular file */
        hFind = FindFirstFile(fileName, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            lastError = GetLastError();
            _ftprintf(stderr, _TEXT("du: ERROR getting handle for file ``%s'': "), fileName);
            displayErrorText(lastError);
            _fputts(_TEXT(""), stderr);  /* Write end of line */
        }
        else {
            maxdword = (unsigned long)MAXDWORD;  /* Avoid Visual C++ 4.0 warning */
            multiplier = maxdword + 1UL;
            size = findFileData.nFileSizeHigh * multiplier + findFileData.nFileSizeLow;
            FindClose(hFind);
            if (displayRegularFilesAlso || isTopLevel) {
                printFileSize(fileName, size);
            }
        }
    }
    return size;
}
