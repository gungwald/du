
#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               2
#define VERSION_MINOR               0
#define VERSION_REVISION            0
#define VERSION_BUILD               BUILD_NUMBER

#define VER_FILE_DESCRIPTION_STR    "Calculates used disk space"
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)    \

#define VER_COMPANY_NAME_STR        "Altered Mechanism"
#define VER_PRODUCTNAME_STR         "Disk Usage"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   "du.exe"
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR           "Copyright (C) 2004, 2021 William L Chatfield"

#ifdef _DEBUG
/*
#define VER_VER_DEBUG             VS_FF_DEBUG
*/
#define VER_VER_DEBUG             0x1L
#else
#define VER_VER_DEBUG             0x0L
#endif

/*
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                VFT_APP
*/
#define VER_FILEOS                  0x40004L
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                0x1L

