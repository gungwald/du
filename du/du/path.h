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

extern Path  *initPath(const TCHAR *path);
extern void  freePath(Path *path);
extern Path  *buildPath(Path *leftPath, const TCHAR *rightPath);
extern TCHAR *getAbsolutePath(Path *path);
extern TCHAR *getOriginalPath(Path *path);
extern void  dumpPath(Path *path);
extern Path  *dirname(const Path *path);
extern TCHAR *basename(Path *path);

#endif /* PATH_H_ */
