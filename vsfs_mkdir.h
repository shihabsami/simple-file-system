#ifndef VSFS_MKDIR_H
#define VSFS_MKDIR_H

#include "vsfs_helpers.h"

int vsfs_mkdir(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "%s Arguments for command \"mkdir\", expected: 2, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], id_path = argv[3];
    std::fstream fs_file;

    // Open the FS file in read and append mode
    int err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::app);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Given ID name may not end with a '/' but the FS always has dirs ending with '/'
    size_t delim_position = id_path.find_first_of(PATH_SEPARATOR);
    if (delim_position == std::string::npos || id_path.at(id_path.size() - 1) != PATH_SEPARATOR)
    {
        // Append '/' if not present in the dir name
        id_path += PATH_SEPARATOR;
    }

    // Verify whether the ID already exists
    std::string fs_line;
    while (read_line(fs_file, fs_line))
    {
        if (fs_line.front() == DIR_RECORD_IDENTIFIER)
        {
            if (fs_line.substr(1) == id_path)
            {
                fprintf(stderr, "%s ID already exists\n", VSFS_ERROR_PREFIX);
                return EXIT_FAILURE;
            }
        }
    }

    try
    {
        // Seek to the end of file to append the new dir record
        fs_file.seekg(0, std::ios::end);
        fs_file << DIR_RECORD_IDENTIFIER << id_path << '\n';
    }
    catch (std::ios::failure& failure)
    {
        // If writing to FS failed
        fprintf(stderr, "%s Failed to add dir to FS: %s",
            VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return EIO;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_MKDIR_H
