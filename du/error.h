#ifndef ERROR_HANDLING_H_BLAH
#define ERROR_HANDLING_H_BLAH 

#include <stdbool.h>
#include <tchar.h>
#include <errno.h>
#include <windows.h>

extern void writeError(errno_t errorCode, const _TCHAR* message, const _TCHAR* object);
extern void writeError2(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR* object2);
extern void writeError3(errno_t errorCode, const _TCHAR* message, const _TCHAR* object1, const _TCHAR* object2, const _TCHAR* object3);
extern void writeLastError(DWORD lastError, const _TCHAR* message, const _TCHAR* object);
extern void writeLastError2(DWORD lastError, const TCHAR *message, const TCHAR *object1, const TCHAR *object2);

#endif

