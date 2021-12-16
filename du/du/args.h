#pragma once

#include <tchar.h>
#include "list.h"

extern bool displayRegularFilesAlso;
extern bool displayBytes;
extern bool summarize;
extern bool humanReadable;

extern List *setSwitches(int argc, TCHAR *argv[]);

