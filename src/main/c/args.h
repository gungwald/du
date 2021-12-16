#pragma once

#include <wchar.h>
#include "list.h"

extern bool displayRegularFilesAlso;
extern bool displayBytes;
extern bool summarize;
extern bool humanReadable;

extern List *setSwitches(int argc, const wchar_t *argv[]);

