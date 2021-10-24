#ifndef VSFS_RMDIR_H
#define VSFS_RMDIR_H

#include "vsfs_helpers.h"

int vsfs_rmdir(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "%s Arguments for command \"rmdir\", expected 2, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], id_path = argv[3];
    std::fstream fs_file;
    bool is_compressed{};

    // Open the FS file in both read and write mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in | std::ios::out);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Given ID name may not end with a '/' but the FS always has dirs ending with '/'
    size_t delim_position = id_path.find_first_of(PATH_SEPARATOR);
    if (delim_position == std::string::npos || id_path.at(id_path.size() - 1) != PATH_SEPARATOR)
    {
        // Append '/' if not present in the dir name
        id_path += PATH_SEPARATOR;
    }

    // Delete the ID
    if (!delete_dir(fs_file, id_path))
    {
        fprintf(stderr, "%s ID could not be found \"%s\"\n",
            VSFS_ERROR_PREFIX, id_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_RMDIR_H
