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

extern Path  *path_init(const TCHAR *path);
extern void   path_free(Path *path);
extern Path  *path_append(Path *leftPath, const TCHAR *rightPath);
extern TCHAR *path_getAbsolute(Path *path);
extern TCHAR *path_getAbsoluteRaw(Path *path);
extern TCHAR *path_getOriginal(Path *path);
extern TCHAR *path_getOriginalRaw(Path *path);
extern void   path_dump(Path *path);
extern Path  *path_getParentDirectory(const Path *path);
extern TCHAR *path_getUnqualifiedName(Path *path);
extern TCHAR *dirname(TCHAR *path);

#endif /* PATH_H_ */
