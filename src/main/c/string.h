#ifndef STRING_UTILS_H_AZAZA
#define STRING_UTILS_H_AZAZA

#include <stdbool.h>
#include <string.h>
#include <wchar.h>

extern wchar_t *concat(const wchar_t *s, const wchar_t *t);
extern wchar_t *concat3(const wchar_t *s, const wchar_t *t,const wchar_t *u);
extern wchar_t *concat4(const wchar_t *s, const wchar_t *t, const wchar_t *u, const wchar_t *v);
extern wchar_t *replaceAll(wchar_t *in, wchar_t from, wchar_t to);
extern char *convertToUtf8(const wchar_t *s);
extern char **convertAllToUtf8(int count, const wchar_t *strs[]);
extern bool startsWith(const wchar_t *s, const wchar_t *prefix);
extern bool endsWith(const wchar_t *s, const wchar_t *suffix);
extern bool endsWithChar(const wchar_t *s, wchar_t c);
extern bool stringContains(const wchar_t *container, const wchar_t *value);
extern wchar_t *createStringCopy(const wchar_t *s);
extern wchar_t *toLowerCase(const wchar_t *s);

#endif

