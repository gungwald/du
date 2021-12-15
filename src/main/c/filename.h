/**
 * path.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef FILENAME_H
#define FILENAME_H

#include <stdbool.h>
#include <wchar.h>
#include "list.h"
#include "string.h"

#define DIR_SEPARATOR L"\\"
#define EXTENDED_LENGTH_PATH_PREFIX L"\\\\?\\"

extern wchar_t *getAbsolutePath(const wchar_t *path);
extern wchar_t *getSimpleName(const wchar_t *path);
extern wchar_t *getParentPath(const wchar_t *path);
extern unsigned long getFileSize(wchar_t *path);
extern List *listDirContents(const wchar_t *path);
extern bool isFile(const wchar_t *path);
extern bool isDirectory(const wchar_t *path);
extern bool isGlob(const wchar_t *path);
extern bool isAbsolutePath(const wchar_t *path);

#endif

