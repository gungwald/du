/*
 * path.c
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */
#include <stdio.h>
#include <stdlib.h>
#include "path.h"
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
static Path  *allocatePath();
static TCHAR *standardizePath(TCHAR *path);
static TCHAR *skipPrefix(TCHAR *path);

Path *path_Init(const TCHAR *name)
{
	Path *path;

	path = (Path *) malloc(sizeof(Path));
	if (path == NULL) {
		writeError(errno, _T("failed to allocate memory for Path variable"), name);
		exit(EXIT_FAILURE);
	}
	else {
		path->original = standardizePath(_tcsdup(name));
		path->absolute = queryForAbsolutePath(path->original);
	}
	return path;
}

TCHAR *standardizePath(TCHAR *path)
{
	return replaceAll(path, _TEXT('/'), _TEXT('\\'));	/* Make sure all separators are backslashes. */
}

void path_Delete(Path *path)
{
	free(path->absolute);
	free(path->original);
	free(path);
}

/* Returns new Path object which must be deleted. */
Path *path_Append(Path *leftPath, const TCHAR *rightPath)
{
	Path *result;
	TCHAR *standardizedRightPath;
	size_t originalLength;
	size_t absoluteLength;

	result = allocatePath();
	standardizedRightPath = standardizePath(_tcsdup(rightPath));
	originalLength = _tcslen(leftPath->original);
	absoluteLength = _tcslen(leftPath->absolute);
    if (originalLength == 0) {
        result->original = _tcsdup(standardizedRightPath);
    }
    else if (leftPath->original[originalLength - 1] == _T('\\')) {
		result->original = concat(leftPath->original, standardizedRightPath);
	}
	else {
		result->original = concat3(leftPath->original, DIR_SEPARATOR, standardizedRightPath);
	}
	if (leftPath->absolute[absoluteLength - 1] == _T('\\')) {
		result->absolute = concat(leftPath->absolute, standardizedRightPath);
	}
	else {
		result->absolute = concat3(leftPath->absolute, DIR_SEPARATOR, standardizedRightPath);
	}
	free(standardizedRightPath);
	return result;
}

TCHAR *path_GetAbsolute(Path *path)
{
	return skipPrefix(path->absolute);
}

TCHAR *path_GetOriginal(Path *path)
{
	return skipPrefix(path->original);
}

TCHAR *path_GetAbsoluteRaw(Path *path)
{
	return path->absolute;
}

TCHAR *path_GetOriginalRaw(Path *path)
{
	return path->original;
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
Path *allocatePath()
{
	Path *path;

	path = (Path *) malloc(sizeof(Path));
	if (path == NULL) {
		writeError(errno, _T("failed to allocate memory"), _T("Path"));
		exit(EXIT_FAILURE);
	}
	return path;
}

/* Result must be freed. */
TCHAR* prefixForExtendedLengthPath(const TCHAR *path) {
	return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

void path_Dump(Path *path)
{
	_tprintf(_T("{ original=%s absolute=%s }\n"), path->original, path->absolute);
}

/**
 * Returns a newly allocated string that must be free'd.
 */
Path *path_GetParentDirectory(const Path *path)
{
	Path *dir;

	dir = allocatePath();
	dir->original = dirname(path->original);
	dir->absolute = dirname(path->absolute);
	return dir;
}

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

TCHAR *path_GetUnqualifiedName(Path *path)
{
	return _tcsdup(_tcsrchr(path->absolute, _T('\\')) + 1);
}

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
