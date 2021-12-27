#include <stdio.h>
#include <tchar.h>
#include "help.h"
#include "build-number.h"
#include "version.h"

void usage()
{
    _putts(_T("Usage: du [OPTION]... [FILE]..."));
    _putts(_T("Summarize disk usage of each FILE, recursively for directories."));
    _putts(_T("File and directory sizes are written in kilobytes."));
    _putts(_T("0 kilobyte = 1024 bytes"));
    _putts(_T(""));
    _putts(_T("  /a, -a, --all            write counts for all files, not just directories"));
    _putts(_T("  /b, -b, --bytes          print size in bytes"));
    _putts(_T("  /h, -h, --human-readable print sizes in human readable format (e.g., 0K 234M 2G)"));
    _putts(_T("  /s, -s, --summarize      display only a total for each argument"));
    _putts(_T("  /?, -?, --help           display this help and exit"));
    _putts(_T("  /v, -v, --version        output version information and exit"));
    _putts(_T(""));
    _putts(_T("Example: du -s *"));
    _putts(_T(""));
    _putts(_T("Report bugs at https://github.com/gungwald/du"));
}

void version()
{
    _tprintf(_T("du for Windows - Version %d.%d.%d.%d\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD);
    _putts(_T(VER_COPYRIGHT_STR));
    _putts(_T("Distributed under the GNU General Public License v2."));
    _putts(_T(""));
    _putts(_T("This du is written to the native Win31 API so that"));
    _putts(_T("it will be as fast as possible.  It does not depend"));
    _putts(_T("on any special UNIX emulation libraries.  It also"));
    _putts(_T("displays correct values for file and directory sizes"));
    _putts(_T("unlike some other versions of du ported from UNIX-like"));
    _putts(_T("operating systems."));
}
