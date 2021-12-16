/*
 * path.c
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "file.h"
#include "string-utils.h"
#include "error-handling.h"
#include "trace.h"

static TCHAR *prefixForExtendedLengthPath(const TCHAR *path);
static TCHAR *queryForAbsolutePath(const TCHAR *path);
static File  *allocateFile();
static TCHAR *slashToBackslash(const TCHAR *path);
static TCHAR *skipPrefix(TCHAR *path);

File *new_File(const TCHAR *name)
{
	File *f;
	TCHAR *absPath;

	f = (File *) malloc(sizeof(File));
	if (f == NULL) {
		writeError(errno, _T("failed to allocate memory for File variable"), name);
		exit(EXIT_FAILURE);
	}
	else {
		f->path = slashToBackslash(name);
		absPath = queryForAbsolutePath(f->path);
		f->extendedLengthAbsolutePath = prefixForExtendedLengthPath(absPath);
		f->type = FILETYPE_UNSET;
		free(absPath);
	}
	return f;
}

/* Returns new Path object which must be deleted. */
File *new_FileWithChild(const File *parent, const TCHAR *child)
{
	File *result;

	result = allocateFile();
	result->path = slashToBackslash(child);
	result->extendedLengthAbsolutePath = concat3(parent->extendedLengthAbsolutePath, DIR_SEPARATOR, result->path);
	result->type = FILETYPE_UNSET;
	return result;
}

/* Result should be freed. */
static TCHAR *slashToBackslash(const TCHAR *path)
{
	return replaceAll(_tcsdup(path), _TEXT('/'), _TEXT('\\'));	/* Make sure all separators are backslashes. */
}

void freeFile(File *f)
{
	free(f->path);
	free(f->extendedLengthAbsolutePath);
	free(f);
}

TCHAR *getAbsolutePath(const File *f)
{
	return skipPrefix(f->extendedLengthAbsolutePath);
}

TCHAR *getPath(const File *f)
{
	return f->path;
}

TCHAR *getAbsolutePathExtLength(const File *f)
{
	return f->extendedLengthAbsolutePath;
}

/* Result must be freed. */
TCHAR *queryForAbsolutePath(const TCHAR *path) {
	TCHAR *absolutePath = NULL;
	DWORD requiredBufferSize;
	DWORD returnedPathLength;

	TRACE_ENTER(_T("queryForAbsolutePath"), _T("path"), path);

	/* Ask for the size of the buffer needed to hold the absolute path. */
	requiredBufferSize = GetFullPathName(path, 0, NULL, NULL);
	if (requiredBufferSize == 0) {
		writeLastError(GetLastError(), _T("failed to get buffer size required for file name"), path);
		exit(EXIT_FAILURE);
	}
	else {
		absolutePath = (TCHAR*) malloc(requiredBufferSize * sizeof(TCHAR));
		if (absolutePath == NULL) {
			writeError(errno, _T("memory allocation failed for absolute path of"), path);
			exit(EXIT_FAILURE);
		}
		else {
			returnedPathLength = GetFullPathName(path, requiredBufferSize, absolutePath, NULL);
			if (returnedPathLength == 0) {
				writeLastError(GetLastError(), _T("failed to get full path name for "), path);
				exit(EXIT_FAILURE);
			}
			else if (returnedPathLength >= requiredBufferSize) {
				writeLastError(GetLastError(), _T("buffer was not big enough for "), path);
				exit(EXIT_FAILURE);
			}
		}
	}
	TRACE_RETURN(_T("queryForAbsolutePath"), absolutePath);
	return absolutePath;
}


/* Result must be freed. */
File *allocateFile()
{
	File *path;

	path = (File *) malloc(sizeof(File));
	if (path == NULL) {
		writeError(errno, _T("failed to allocate memory"), _T("File"));
		exit(EXIT_FAILURE);
	}
	return path;
}

/* Result must be freed. */
TCHAR* prefixForExtendedLengthPath(const TCHAR *path) {
	return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

void printPath(const File *path)
{
	_tprintf(_T("{ %s %s %d }\n"), path->path, path->extendedLengthAbsolutePath, path->type);
}

/**
 * Returns a newly allocated object that must be free'd.
 */
File *getParent(const File *f)
{
	File *parent;

	parent = allocateFile();
	parent->extendedLengthAbsolutePath = dirname(getAbsolutePathExtLength(f));
	parent->path = _tcsdup(getAbsolutePath(parent));
	parent->type = FILETYPE_DIRECTORY;
	return parent;
}

/* Result must be freed. */
TCHAR *dirname(const TCHAR *path)
{
	TCHAR *dir;
	TCHAR *lastBackslashPointer;

	dir = _tcsdup(path);
	lastBackslashPointer = _tcsrchr(dir, '\\');	/* Find the last backslash. */
	if (lastBackslashPointer != NULL) {
		*lastBackslashPointer = '\0';			/* Terminate the string at the last backslash. */
	}
	else {
		*dir = '\0';							/* Make it an empty string. */
	}
	return dir;
}

/* Returns pointer which should not be freed. */
TCHAR *getName(const File *f)
{
	return _tcsrchr(getAbsolutePath(f), _T('\\')) + 1;
}

/* Returns pointer which should not be freed. */
TCHAR *skipPrefix(TCHAR *path)
{
    size_t prefixLength;
    TCHAR *result;

    result = path;
    prefixLength = _tcslen(EXTENDED_LENGTH_PATH_PREFIX);
    if (prefixLength > 0) {
        if (_tcsncmp(path, EXTENDED_LENGTH_PATH_PREFIX, prefixLength) == 0) {
            result = path + prefixLength;
        }
    }
    return result;
}

unsigned long getLength(File *f)
{
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	unsigned long size = 0;
	unsigned long multiplier;
	unsigned long maxDWORD;

	TRACE_ENTER_CALLBACK(__func__, _T("path"), printPath, f);

	findHandle = FindFirstFile(getAbsolutePathExtLength(f), &fileProperties);
	if (findHandle == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for file"), getAbsolutePath(f));
	}
	else {
		maxDWORD = (unsigned long) MAXDWORD; /* Avoid Visual C++ 4.0 warning */
		multiplier = maxDWORD + 1UL;
		size = fileProperties.nFileSizeHigh * multiplier + fileProperties.nFileSizeLow;
		FindClose(findHandle);
	}
	TRACE_RETURN_ULONG(__func__, size);
	return size;
}

List *listFiles(const File *f)
{
	HANDLE findHandle;
	WIN32_FIND_DATA fileProperties;
	TCHAR *searchPattern;
	bool moreDirectoryEntries;
	TCHAR *entry;
	File *fileEntry;
	List *files;
	DWORD lastError;

	files = list_init();
	if (isGlob(f->path)) {
		searchPattern = f->extendedLengthAbsolutePath;
	}
	else {
		searchPattern = concat3(getAbsolutePathExtLength(f), DIR_SEPARATOR, _T("*"));

	}
	if ((findHandle = FindFirstFile(searchPattern, &fileProperties)) == INVALID_HANDLE_VALUE) {
		writeLastError(GetLastError(), _T("failed to get handle for file search pattern"), searchPattern);
	}
	else {
		moreDirectoryEntries = true;
		while (moreDirectoryEntries) {
			entry = fileProperties.cFileName;
			if (_tcscmp(entry, _T(".")) != 0 && _tcscmp(entry, _T("..")) != 0) {
				if ((fileEntry = new_FileWithChild(f, entry)) != NULL) {
					list_append(files, fileEntry);
				}
			}
			if (!FindNextFile(findHandle, &fileProperties)) {
				if ((lastError = GetLastError()) == ERROR_NO_MORE_FILES) {
					moreDirectoryEntries = false;
				}
				else {
					writeLastError(lastError, _T("failed to get next file search results"), searchPattern);
				}
			}
		}
		FindClose(findHandle); /* Only close it if it got opened successfully */
	}
	free(searchPattern);
	return files;
}

enum FileType populateFileType(File *f)
{
	DWORD fileAttributes;

	if (f->type == FILETYPE_UNSET) {
		if (isGlob(getAbsolutePath(f))) {
			f->type = FILETYPE_GLOB;
		} else {
			fileAttributes = GetFileAttributes(getAbsolutePathExtLength(f));
			if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
				writeLastError(GetLastError(), _T("failed to get attributes for file"), getAbsolutePath(f));
			}
			else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				f->type = FILETYPE_DIRECTORY;
			}
			else {
				f->type = FILETYPE_FILE;
			}
		}
	}
	return f->type;
}

bool isFile(File *f)
{
	if (f->type == FILETYPE_UNSET) {
		populateFileType(f);
	}
	return f->type == FILETYPE_FILE;
}

bool isDirectory(File *f)
{
	if (f->type == FILETYPE_UNSET) {
		populateFileType(f);
	}
	return f->type == FILETYPE_DIRECTORY;
}

bool isGlobPattern(File *f)
{
	if (f->type == FILETYPE_UNSET) {
		populateFileType(f);
	}
	return f->type == FILETYPE_GLOB;
}
