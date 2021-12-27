// These tests show that GetFullPathName only works with an extended length prefix \\?\
// if the given path is already an absolute path, which defeats the purpose of the
// function because GetFullPathName is supposed to determine the absolute path.
//

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

static TCHAR *queryForAbsolutePath(TCHAR *path);

int main()
{
    TCHAR *paths[] = { 
        (TCHAR *) _T("C:\\"),
        (TCHAR *) _T("\\\\?\\C:\\"),
        (TCHAR *) _T("\\\\?\\.\\"),
        (TCHAR *) _T("\\\\?\\Release"),
        (TCHAR *) _T("\\\\?\\GetFullPathName-tests.cpp"),
        (TCHAR *) _T("\\\\?\\..\\du\\du.c"),
        (TCHAR *) _T(".\\"),
        (TCHAR *) _T("Release"),
        (TCHAR *) _T("GetFullPathName-tests.cpp"),
        (TCHAR *) _T("..\\du\\du.c"),
        NULL
    };
    for (int i = 0; paths[i]; i++) {
        _tprintf(_T("queryForAbsolutePath(%s)=%s\n"), paths[i], queryForAbsolutePath(paths[i]));
    }
}

TCHAR *queryForAbsolutePath(TCHAR *path)
{
    DWORD sizeOfRequiredBuffer = GetFullPathName(path, 0, NULL, NULL);
    TCHAR *buffer = (TCHAR *) malloc(sizeOfRequiredBuffer * sizeof(TCHAR));
    if (buffer) {
		TCHAR **basename = NULL;
        if (!GetFullPathName(path, sizeOfRequiredBuffer, buffer, basename)) {
            _ftprintf(stderr, _T("Failed to get absolute path name: GetLastError=%d\n"), GetLastError());
        }
    }
    else {
        _ftprintf(stderr, _T("Failed to determine size of buffer: GetLastError=%d\n"), GetLastError());
    }
    return buffer;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
