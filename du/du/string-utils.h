#ifndef STRING_UTILS_H_AZAZA
#define STRING_UTILS_H_AZAZA

#include <stdbool.h>
#include <string.h>
#include <tchar.h>

extern bool isGlob(const _TCHAR* fileName);
extern _TCHAR* concat(const _TCHAR* left, const _TCHAR* right);
extern _TCHAR* concat3(const _TCHAR* first, const _TCHAR* second, const _TCHAR* third);
extern _TCHAR *replaceAll(_TCHAR *s, _TCHAR searchFor, _TCHAR replaceWith);
extern char *convertWideCharStringToUtf8(const wchar_t *wideCharString);
extern char **convertTcharStringArrayToUtf8(int argc, TCHAR *argv[]);
extern void freeStringArray(int elementCount, char *array[]);

#endif
