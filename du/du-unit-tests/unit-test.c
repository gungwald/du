#include <tchar.h>
#include "munit.h"
#include "../du/File.h"

TCHAR *programName;

MunitResult testNewFile(const MunitParameter params[], void *user_data_or_fixture)
{
	File *f = new_File(_T("."));
	munit_assert_true(_tcscmp(getPath(f), _T(".")) == 0);
    return MUNIT_OK;
}

MunitTest tests[] = {
  {
    "/testNewFile", /* name */
    testNewFile, /* test */
    NULL, /* setup */
    NULL, /* tear_down */
    MUNIT_TEST_OPTION_NONE, /* options */
    NULL /* parameters */
  },
  /* Mark the end of the array with an entry where the test
   * function is NULL */
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
  "/suite", /* name */
  tests, /* tests */
  NULL, /* suites */
  1, /* iterations */
  MUNIT_SUITE_OPTION_NONE /* options */
};

int main (int argc, const char* argv[]) {
    programName = argv[0];
    return munit_suite_main(&suite, NULL, argc, argv);
}

