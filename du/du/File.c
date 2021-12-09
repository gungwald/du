/*
 * path.c
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */
#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "string-utils.h"
#include "error-handling.h"
#include "trace.h"

#ifdef UNICODE
#define EXTENDED_LENGTH_PATH_PREFIX _T("\\\\?\\")
#else
#define EXTENDED_LENGTH_PATH_PREFIX ""
#endif

static TCHAR *prefixForExtendedLengthPath(const TCHAR *path);
static TCHAR *queryForAbsolutePath(const TCHAR *path);
static File  *allocateFile();
static TCHAR *slashToBackslash(TCHAR *path);
static TCHAR *skipPrefix(TCHAR *path);

File *new_File(const TCHAR *name)
{
	File *f;
	TCHAR *path;

	f = (File *) malloc(sizeof(File));
	if (f == NULL) {
		writeError(errno, _T("failed to allocate memory for File variable"), name);
		exit(EXIT_FAILURE);
	}
	else {
		path = slashToBackslash(prefixForExtendedLengthPath(name));
		f->extendedLengthAbsolutePath = queryForAbsolutePath(path);
		f->path = f->extendedLengthAbsolutePath + (_tcslen(f->extendedLengthAbsolutePath) - _tcslen(path));
		free(path);
	}
	return f;
}

/* Returns new Path object which must be deleted. */
File *new_FileWithChild(const File *parent, const TCHAR *child)
{
	File *result;
	TCHAR *standardizedChild;
	size_t parentLength;

	result = allocateFile();
	standardizedChild = slashToBackslash(_tcsdup(child));
	result->extendedLengthAbsolutePath = concat3(parent->extendedLengthAbsolutePath, DIR_SEPARATOR, standardizedChild);
	result->path = result->extendedLengthAbsolutePath + _tcslen(parent->path);
	free(standardizedChild);
	return result;
}

TCHAR *slashToBackslash(TCHAR *path)
{
	return replaceAll(path, _TEXT('/'), _TEXT('\\'));	/* Make sure all separators are backslashes. */
}

void delete_File(File *f)
{
	free(f->extendedLengthAbsolutePath);
	free(f);
}

TCHAR *getAbsolutePath(File *f)
{
	return skipPrefix(f->extendedLengthAbsolutePath);
}

TCHAR *getPath(File *f)
{
	return f->path;
}

TCHAR *getAbsolutePathExtLen(File *f)
{
	return f->extendedLengthAbsolutePath;
}

/* Result must be freed. */
TCHAR *queryForAbsolutePath(const TCHAR *path) {
	TCHAR *absolutePath = NULL;
	TCHAR *prefixedPath = NULL;
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
			}
			else if (returnedPathLength >= requiredBufferSize) {
				writeLastError(GetLastError(), _T("buffer was not big enough for "), path);
				exit(EXIT_FAILURE);
			}
		}
	}
	if (absolutePath != NULL) {
		prefixedPath = prefixForExtendedLengthPath(absolutePath);
		free(absolutePath);
	}
	TRACE_RETURN(_T("queryForAbsolutePath"), prefixedPath);
	return prefixedPath;
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

void path_dump(File *path)
{
	_tprintf(_T("{ extLenPath=%s absolute=%s }\n"), path->extLenPath, path->extLenAbsolutePath);
}

/**
 * Returns a newly allocated object that must be free'd.
 */
File *getParent(const File *f)
{
	File *parent;

	parent = allocateFile();
	parent->extLenPath = dirname(getAbsolutePath(f));
	parent->extLenAbsolutePath = NULL;
	return parent;
}

/* Result must be freed. */
TCHAR *dirname(TCHAR *path)
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
TCHAR *getName(File *f)
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
