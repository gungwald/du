/*
 * path.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef FILE_H_BLAHBLAHBLAH
#define FILE_H_BLAHBLAHBLAH

#include <stdbool.h>
#include <tchar.h>
#include "list.h"

#define DIR_SEPARATOR _T("\\")

#ifdef UNICODE
#define EXTENDED_LENGTH_PATH_PREFIX _T("\\\\?\\")
#else
#define EXTENDED_LENGTH_PATH_PREFIX ""
#endif

enum FileType {FILETYPE_UNSET, FILETYPE_FILE, FILETYPE_DIRECTORY, FILETYPE_GLOB};

struct file
{
	TCHAR *extendedLengthAbsolutePath;
	TCHAR *path; /* The original path provided when this object was created. */
	enum FileType type;
};

typedef
	struct file
	File;

extern File *new_File(const TCHAR *pathName);
extern File *new_FileWithChild(const File *parent, const TCHAR *child);
extern void freeFile(File *f);
extern TCHAR *getAbsolutePathExtLength(const File *f);
extern TCHAR *getAbsolutePath(const File *f);
extern TCHAR *getPath(const File *f);
extern void  printPath(const File *f);
extern File *getParent(const File *f);
extern TCHAR *getName(const File *f);
extern TCHAR *dirname(const TCHAR *f);
extern unsigned long getLength(File *f);
extern List *listFiles(const File *f);
extern bool isFile(File *f);
extern bool isDirectory(File *f);
extern bool isGlobPattern(File *f);

#endif
