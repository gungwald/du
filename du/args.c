#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>
#include <windows.h>
#include "args.h"
#include "string.h"
#include "help.h"

/* If Microsoft's C compiler is being used, then include the local getopt.h
   because Microsoft does not provide one. Otherwise include the system
   getopt.h */
#ifdef _MSC_FULL_VER
#include "getopt.h"
#else
#include <getopt.h>
#endif

bool displayRegularFilesAlso = false;
bool displayBytes = false;
bool summarize = false;
bool humanReadable = false;

static const wchar_t *programName;

List *setSwitches(int argc, const wchar_t *argv[])
{
    int optionChar;
    List *remainingArguments;
    struct option longOptions[] = {
        {"help",           no_argument, NULL, '?'},
        {"version",        no_argument, NULL, 'v'},
        {"all",            no_argument, NULL, 'a'},
        {"bytes",          no_argument, NULL, 'b'},
        {"summarize",      no_argument, NULL, 's'},
        {"human-readable", no_argument, NULL, 'h'},
        {0,                0,           0,     0 }
    };
    int optionIndex = 0;
    const int END_OF_OPTIONS = -1;
    char **arguments;
    /* optind - system sets to index of next argument in argv. */

    programName = argv[0];
    arguments = convertAllToUtf8(argc, argv);

    while ((optionChar = getopt_long(argc, arguments, "?vabsh", longOptions, &optionIndex)) != END_OF_OPTIONS) {
        switch (optionChar) {
        case '?':
            usage();
            exit(EXIT_SUCCESS);	/* The Linux man page says to exit after printing help. */
            break;
        case 'v':
            version();
            exit(EXIT_SUCCESS);	/* The Linux man page says to exit after printing the version. */
            break;
        case 'a':
            displayRegularFilesAlso = true;
            break;
        case 'b':
            displayBytes = true;
            break;
        case 's':
            summarize = true;
            break;
        case 'h':
            humanReadable = true;
            break;
        default:
            fwprintf(stderr, L"%ls: getopt_long returned unrecognized option: %c\n", programName, optionChar);
            exit(EXIT_FAILURE);
        }
    }

    if (displayRegularFilesAlso && summarize) {
        fwprintf(stderr, L"%ls: ERROR with arguments: cannot both summarize and show all entries\n", programName);
        exit(EXIT_FAILURE);
    }

    remainingArguments = initList();
    while (optind < argc) {
        appendListItem(&remainingArguments, wcsdup(argv[optind++]));
    }
    return remainingArguments;
}
