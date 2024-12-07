#ifndef HEAP_0202020202
#define HEAP_0202020202

extern wchar_t* allocateString(size_t sizeInCharsIncludingNull);
extern wchar_t* allocateStringBytes(size_t sizeInBytes);
extern char** allocateUTF8PointerArray(size_t pointerCount);
extern void freeString(wchar_t* s);

#endif
