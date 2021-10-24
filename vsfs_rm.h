#ifndef VSFS_RM_H
#define VSFS_RM_H

#include "vsfs_helpers.h"

int vsfs_rm(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "%s Arguments for command \"rm\", expected 2, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], if_path = argv[3];
    std::fstream fs_file;
    bool is_compressed{};

    // Open the FS file in both read and write mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in | std::ios::out);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Delete the IF
    if (!delete_record(fs_file, if_path))
    {
        fprintf(stderr, "%s IF could not be found \"%s\"\n",
            VSFS_ERROR_PREFIX, if_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_RM_H
