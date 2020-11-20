/*
 * path.h
 *
 *  Created on: Nov 14, 2020
 *      Author: bill
 */

#ifndef PATH_H_BLAHBLAHBLAH
#define PATH_H_BLAHBLAHBLAH

#include <tchar.h>

#define DIR_SEPARATOR _T("\\")

struct PathStruct
{
	TCHAR *original;
	TCHAR *absolute;
};

typedef
	struct PathStruct
	Path;

extern Path  *new_Path(const TCHAR *path);
extern void  delete_Path(Path *path);
extern Path  *pathAppend(Path *leftPath, const TCHAR *rightPath);
extern TCHAR *pathGetAbsolute(Path *path);
extern TCHAR *pathGetAbsoluteRaw(Path *path);
extern TCHAR *pathGetOriginal(Path *path);
extern TCHAR *pathGetOriginalRaw(Path *path);
extern void  pathDump(Path *path);
extern Path  *pathDirName(const Path *path);
extern TCHAR *pathBaseName(Path *path);
extern TCHAR *dirname(TCHAR *path);

#endif /* PATH_H_ */
