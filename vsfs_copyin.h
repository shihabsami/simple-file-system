#ifndef VSFS_COPYIN_H
#define VSFS_COPYIN_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

#include <fstream>
#include <sstream>

int vsfs_copyin(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 5)
    {
        fprintf(stderr, "%s Arguments for command \"copyin\", expected: 3, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], ef_path = argv[3], if_path = argv[4];
    std::fstream fs_file, ef_file;
    int err_code = EXIT_FAILURE;

    // Open FS file in both read & write mode
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
        return err_code;

    // Open EF file in read mode, throws error if file does not exist
    if ((err_code = open_ef(ef_path, ef_file, std::ios::in, true)) != EXIT_SUCCESS)
        return err_code;

    // Check whether the given path for IF is valid
    if (!is_internal_path_valid(if_path))
    {
        fprintf(stderr, "%s Invalid IF provided: %s\n", VSFS_ERROR_PREFIX, if_path.c_str());
        return EXIT_FAILURE;
    }

    // Delete the record first, if existing
    delete_record(fs_file, if_path);

    // Seek to the end of file to append the new file record
    fs_file.seekg(0, std::ios::end);

    try
    {
        // Add new record entry to FS
        fs_file << FILE_RECORD_IDENTIFIER << if_path << '\n';

        // Write from EF to IF
        std::string ef_line;
        while (read_line(ef_file, ef_line))
        {
            // Truncating (?) the file to 255 characters
            size_t size = ef_line.size();
            if (size > MAXIMUM_RECORD_LENGTH)
            {
                ef_line.resize(MAXIMUM_RECORD_LENGTH);
                ef_line.append("\n");
            }
            else if (size == MAXIMUM_RECORD_LENGTH || (size > 0 && ef_line.at(size - 1) != '\n'))
            {
                ef_line.append("\n");
            }

            // Write the content one line at a time
            fs_file << RECORD_CONTENT_IDENTIFIER << ef_line;
        }
    }
    catch (const std::fstream::failure& failure)
    {
        // If reading from EF or writing to FS failed
        fprintf(stderr, "%s Failed to write entry EF in FS: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYIN_H
