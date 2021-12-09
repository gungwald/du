/*
 * path.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef FILE_H_BLAHBLAHBLAH
#define FILE_H_BLAHBLAHBLAH

#include <tchar.h>

#define DIR_SEPARATOR _T("\\")

struct file
{
	TCHAR *extendedLengthAbsolutePath;
	TCHAR *path; /* A pointer into extendedLengthAbsolutePath */
};

typedef
	struct file
	File;

extern File *new_File(const TCHAR *pathName);
extern File  *new_FileWithChild(const TCHAR *parent, const TCHAR *child);
extern void   delete_File(File *f);
extern TCHAR *getAbsolutePathExtLen(const File *f);
extern TCHAR *getAbsolutePath(const File *f);
extern TCHAR *getPathExtLen(const File *f);
extern TCHAR *getPath(const File *f);
extern void   printPath(const File *f);
extern File  *getParent(const File *f);
extern TCHAR *getName(const File *f);
extern TCHAR *dirname(TCHAR *f);

#endif /* PATH_H_ */
