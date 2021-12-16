#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "filename.h"
#include "string.h"
#include "error.h"
#include "trace.h"

extern wchar_t *makeExtendedLenPath(const wchar_t *path);
extern wchar_t *makeNormalPath(const wchar_t *path);
extern bool isExtendedLenPath(const wchar_t *path);
static wchar_t *prefixForExtendedLengthPath(const wchar_t *path);
static wchar_t *queryForAbsolutePath(const wchar_t *path);
static File  *allocateFile();
static wchar_t *slashToBackslash(const wchar_t *path);
static wchar_t *skipPrefix(wchar_t *path);

/* Result should be freed. */
static wchar_t *slashToBackslash(const wchar_t *path)
{
    /* Make sure all separators are backslashes. */
    return replaceAll(_tcsdup(path), _TEXT('/'), _TEXT('\\'));
}

/* Result must be freed. */
wchar_t *getAbsolutePath(const wchar_t *path) {
    wchar_t *absolutePath = NULL;
    DWORD reqSize;
    DWORD returnedLen;

    /* Ask for the size of the buffer needed to hold the absolute path. */
    reqSize = GetFullPathName(path, 0, NULL, NULL);
    if (reqSize == 0) {
        writeLastError(GetLastError(), L"failed to get required buf size", path);
        exit(EXIT_FAILURE);
    }
    else {
        absolutePath = (wchar_t *) malloc(reqSize * sizeof(wchar_t));
        if (absolutePath == NULL) {
            writeError(errno, _T("memory alloc failed for abs path of"), path);
            exit(EXIT_FAILURE);
        }
        else {
            returnedLen = GetFullPathName(path, reqSize, absolutePath, NULL);
            if (returnedLen == 0) {
                writeLastError(GetLastError(), L"failed to get full path", path);
                exit(EXIT_FAILURE);
            }
            else if (returnedLen >= reqSize) {
                writeLastError(GetLastError(), L"buffer not big enough", path);
                exit(EXIT_FAILURE);
            }
        }
    }
    return absolutePath;
}

/* Result must be freed. */
wchar_t *makeExtendedLengthPath(const wchar_t *path) {
    return concat(EXTENDED_LENGTH_PATH_PREFIX, path);
}

/* Result must be freed. */
wchar_t *getParentPath(const wchar_t *path)
{
    wchar_t *parent;
    wchar_t *lastBackslashPointer;
    wchar_t absPath;
    size_t parentSize;

    if (isAbsolutePath(path)) {
        absPath = path;
    } else {
        absPath = getAbsolutePath(path);
    }
    lastBackslashPointer = wcsrchr(dir, L'\\');
    parentSize = lastBackslashPointer - path + 1;
    parent = (wchar_t *) malloc(parentSize * sizeof(wchar_t));
    strncpy(parent, parentSize, absPath);
    if (absPath != path) {
        free(absPath);
    }
    return parent;
}

/* STOPPED HERE */

/* Returns pointer which should not be freed. */
wchar_t *getName(const File *f)
{
    return _tcsrchr(getAbsolutePath(f), _T('\\')) + 1;
}

/* Returns pointer which should not be freed. */
wchar_t *skipPrefix(wchar_t *path)
{
    size_t prefixLength;
    wchar_t *result;

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
	wchar_t *searchPattern;
	bool moreDirectoryEntries;
	wchar_t *entry;
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
