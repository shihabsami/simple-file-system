#ifndef VSFS_CONSTANTS_H
#define VSFS_CONSTANTS_H

enum VSFS_commands
{
    LIST,
    COPYIN,
    COPYOUT,
    MKDIR,
    RM,
    RMDIR,
    DEFRAG
};

const char* commands[]{
    "list",
    "copyin",
    "copyout",
    "mkdir",
    "rm",
    "rmdir",
    "defrag"
};

constexpr const char* FS_EXTENSION = "notes";
constexpr const char* GZ_EXTENSION = "gz";
constexpr const char* FS_FIRST_RECORD = "NOTES V1.0";
constexpr unsigned int MAXIMUM_RECORD_LENGTH = 255;
constexpr int ASCII_MAX_VALUE = 127;
constexpr char FILE_RECORD_IDENTIFIER = '@';
constexpr char DIR_RECORD_IDENTIFIER = '=';
constexpr char DELETED_RECORD_IDENTIFIER = '#';
constexpr char RECORD_CONTENT_IDENTIFIER = ' ';
constexpr char PATH_SEPARATOR = '/';
constexpr const char* VSFS_ERROR_PREFIX = "Invalid VSFS:";

#endif // VSFS_CONSTANTS_H
