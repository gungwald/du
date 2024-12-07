#ifndef SRC_MAIN_C_REGISTRY_H_
#define SRC_MAIN_C_REGISTRY_H_

#include <windows.h>
#include <tchar.h>

extern HKEY openRegistryKey(HKEY parentKey, const _TCHAR *keyName);
extern void closeRegistryKey(HKEY key, const _TCHAR *keyName);
extern void setRegistryStringData(HKEY key, const _TCHAR *valueName, const _TCHAR *data);
extern _TCHAR *getRegistryStringData(HKEY key, const _TCHAR *value);
extern void addElementToRegistryUserPath(const _TCHAR *element);

#endif
