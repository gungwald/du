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

extern Path  *path_Init(const TCHAR *path);
extern void   path_Delete(Path *path);
extern Path  *path_Append(Path *leftPath, const TCHAR *rightPath);
extern TCHAR *path_GetAbsolute(Path *path);
extern TCHAR *path_GetAbsoluteRaw(Path *path);
extern TCHAR *path_GetOriginal(Path *path);
extern TCHAR *path_GetOriginalRaw(Path *path);
extern void   path_Dump(Path *path);
extern Path  *path_GetParentDirectory(const Path *path);
extern TCHAR *path_GetUnqualifiedName(Path *path);
extern TCHAR *dirname(TCHAR *path);

#endif /* PATH_H_ */
