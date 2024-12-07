/**
 * path.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef FILENAME_H
#define FILENAME_H

#include <stdbool.h>
#include <stdint.h>     /* int64_t */
#include <wchar.h>
#include "list.h"
#include "string.h"

#define DIR_SEPARATOR L"\\"
#define EXTENDED_LENGTH_PATH_PREFIX L"\\\\?\\"

extern wchar_t *getAbsolutePath(const wchar_t *path);
extern const wchar_t *getSimpleName(const wchar_t *path);
extern wchar_t *getParentPath(const wchar_t *path);
extern int64_t getFileSize(wchar_t *path);
extern List *listFiles(const wchar_t *path);
extern bool isFile(const wchar_t *path);
extern bool isDirectory(const wchar_t *path);
extern bool isGlob(const wchar_t *path);
extern bool isAbsolutePath(const wchar_t *path);
extern wchar_t *buildPath(const wchar_t *dir, const wchar_t *file);
extern bool fileExists(wchar_t *path);
extern wchar_t* makeExtendedLengthPath(const wchar_t *path);
extern wchar_t* makeNormalPath(const wchar_t *path);
extern bool isExtendedLengthPath(const wchar_t *path);
extern wchar_t* slashToBackslash(const wchar_t *path);
extern const wchar_t* skipPrefix(wchar_t *path);

#endif

