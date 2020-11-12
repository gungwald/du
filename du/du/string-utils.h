#ifndef STRING_UTILS_H_AZAZA
#define STRING_UTILS_H_AZAZA

#include <stdbool.h>
#include <string.h>
#include <tchar.h>

extern bool isGlob(const _TCHAR* fileName);
extern _TCHAR* concat(const _TCHAR* left, const _TCHAR* right);
extern _TCHAR* concat3(const _TCHAR* first, const _TCHAR* second, const _TCHAR* third);
extern _TCHAR *replaceAll(_TCHAR *s, _TCHAR searchFor, _TCHAR replaceWith);
extern _TCHAR *dirname(const _TCHAR *path);

#endif