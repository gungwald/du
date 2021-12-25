#ifndef SRC_MAIN_C_REGISTRY_H_
#define SRC_MAIN_C_REGISTRY_H_

#include <windows.h>
#include <tchar.h>

extern _TCHAR *getRegistryStringValueForUpdate(HKEY key, const _TCHAR *value);
extern int appendToUserPath(_TCHAR *newPathElement);

#endif
