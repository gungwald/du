#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include "munit.h"
#include "../du/File.h"
#include "../du/args.h"
#include "../du/string-utils.h"

#pragma warning(disable:4996) /* _CRT_SECURE_NO_WARNINGS */

const TCHAR *programName;
TCHAR *cwd;
TCHAR *testDirName;
TCHAR *testFileName1;
FILE *testFile1;
unsigned long lenTestFile1 = 1000;
File *dot;

static void fail(TCHAR *message)
{
	_tperror(message);
	exit(EXIT_FAILURE);
}

/** Result must be freed. */
static TCHAR *getCurrentDirectory() {
	TCHAR *cwd;
	if ((cwd = _tgetcwd(NULL, 0)) == NULL)
		fail(_T("Failed to get current working directory"));
	return cwd;
}

/** Result must be freed. */
static TCHAR *createTestDirectory() {
	TCHAR *testDirName;
	const int MKDIR_FAILURE = -1;
	if ((testDirName = _ttempnam(NULL, _T("du-unit-test-data."))) == NULL)
		fail(_T("Failed to create temp file name for test directory"));
	if (_tmkdir(testDirName) == MKDIR_FAILURE)
		fail(_T("Failed to make test directory"));
	return testDirName;
}

/** Result must be freed. */
static TCHAR *createTestFile1(TCHAR *dirName) {
	TCHAR *testFileName1;
	FILE *testFile1;
	unsigned long i;

	testFileName1 = concat3(dirName, DIR_SEPARATOR, _T("du-unit-test-file-1"));
	if ((testFile1 = _tfopen(testFileName1, _T("w"))) == NULL) {
		fail(concat(_T("Failed to open test file: "), testFileName1));
	}
	for (i = 0; i < lenTestFile1; i++) {
		fputc('b', testFile1);
	}
	fclose(testFile1);
	return testFileName1;
}

static void *setup(const MunitParameter params[], void *user_data) 
{
	cwd = getCurrentDirectory();
	testDirName = createTestDirectory();
	testFileName1 = createTestFile1(testDirName);
	dot = new_File(_T("."));
	return NULL;
}

static void tearDown(void *fixture) 
{
	_tremove(testFileName1);
	_trmdir(testDirName);
	free(testFileName1);
	free(testDirName);
	freeFile(dot);
}

MunitResult testNewFile(const MunitParameter params[], void *user_data_or_fixture)
{
	File *f = new_File(_T("."));
	munit_assert_true(_tcscmp(f->path, _T(".")) == 0);
	munit_assert_true(_tcscmp(f->extendedLengthAbsolutePath+4, cwd) == 0);
	munit_assert_true(_tcscmp(f->extendedLengthAbsolutePath, concat(EXTENDED_LENGTH_PATH_PREFIX, cwd)) == 0);
	freeFile(f);
	return MUNIT_OK;
}

MunitResult testIsFile(const MunitParameter params[], void *user_data_or_fixture)
{
	File *f = new_File(testFileName1);
	munit_assert_true(isFile(f));
	freeFile(f);
	return MUNIT_OK;
}

MunitResult testGetAbsolutePath(const MunitParameter params[], void *user_data_or_fixture)
{
	File *f = new_File(testFileName1);
	munit_assert_true(_tcscmp(getAbsolutePath(f), testFileName1) == 0);
	TCHAR *name = getName(f);
	freeFile(f);
	File *f2 = new_File(_T("blah"));
	munit_assert_true(_tcscmp(getAbsolutePath(f2), concat3(cwd, DIR_SEPARATOR, _T("blah"))) == 0);
	freeFile(f2);
	return MUNIT_OK;
}

static MunitParameterEnum empty_params[] = {
	{ NULL, NULL },
};

MunitTest tests[] = {
	{ "/testNewFile", testNewFile, setup, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ "/testIsFile", testIsFile, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
	{ "/testGetAbsolutePath", testGetAbsolutePath, NULL, tearDown, MUNIT_TEST_OPTION_NONE, NULL },
	{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
	"/suite", /* name */
	tests, /* tests */
	NULL, /* suites */
	1, /* iterations */
	MUNIT_SUITE_OPTION_NONE /* options */
};

int _tmain(int argc, const TCHAR *argv[]) {
	programName = argv[0];
	char **arguments = convertTcharStringArrayToUtf8(argc, argv);
	int exitCode = munit_suite_main(&suite, NULL, argc, arguments);
	return exitCode;
}

