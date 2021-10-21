#ifndef VSFS_RM_H
#define VSFS_RM_H

#include "vsfs_helpers.h"

int vsfs_rm(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "%s Arguments for command \"rm\", expected: 2, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], if_path = argv[3];
    std::stringstream fs_stream;

    try
    {
        // Open the FS file in read mode
        int err_code = EXIT_FAILURE;
        std::fstream fs_file;
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in)) != EXIT_SUCCESS)
            return err_code;

        // Read the contents into a stringstream and close the file resource since no write is done
        fs_stream << fs_file.rdbuf();
        fs_file.close();

        // Open FS file in read/write/append mode
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
            return err_code;

        // Delete the IF
        if (!delete_record(fs_file, if_path))
        {
            fprintf(stderr, "%s IF could not be found: %s\n", VSFS_ERROR_PREFIX, if_path.c_str());
            return EXIT_FAILURE;
        }
    }
    catch (const std::ios_base::failure& failure)
    {
        // If file could not be opened/read
        fprintf(stderr, "%s Failed to read FS: %s\n",
            VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_RM_H
