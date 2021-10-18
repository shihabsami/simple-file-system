#ifndef VSFS_CONSTANTS_H
#define VSFS_CONSTANTS_H

#include <system_error>

enum VSFS_commands {
    LIST, COPYIN, COPYOUT, MKDIR, RM, RMDIR, DEFRAG, INDEX
};

constexpr const char* FS_EXTENSION = "notes";
constexpr const char* FS_FIRST_RECORD = "NOTES V1.0";
constexpr unsigned int MAXIMUM_RECORD_LENGTH = 255;
constexpr char FILE_RECORD_IDENTIFIER = '@';
constexpr char DIR_RECORD_IDENTIFIER = '=';
constexpr char DELETED_RECORD_IDENTIFIER = '#';
constexpr char RECORD_CONTENT_IDENTIFIER = ' ';
constexpr const char* VSFS_ERROR_PREFIX = "Invalid VSFS:";
constexpr const char* GZIP_ENCODE_PREFIX = "gzip ";
constexpr const char* GZIP_DECODE_PREFIX = "gzip -d ";

#endif // VSFS_CONSTANTS_H
