#ifndef VSFS_COPYIN_H
#define VSFS_COPYIN_H

#include "vsfs_externals.h"
#include "vsfs_helpers.h"
#include "vsfs_constants.h"

#include <fstream>
#include <sstream>

int vsfs_copyin(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 5)
    {
        fprintf(stderr, "%s Arguments for command \"copyin\", expected 3, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], ef_path = argv[3], if_path = argv[4];
    std::fstream fs_file, ef_file;
    bool is_compressed{};

    // Open FS file in both read and write mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in | std::ios::out);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Open EF file in read mode, throws error if file does not exist
    if ((err_code = open_ef(ef_path, ef_file, std::ios::in, true)) != EXIT_SUCCESS)
        return err_code;

    // Check whether the given path for IF is valid
    if (!is_internal_path_valid(if_path, false))
    {
        fprintf(stderr, "%s Invalid IF provided \"%s\"\n", VSFS_ERROR_PREFIX, if_path.c_str());
        return EXIT_FAILURE;
    }

    // Determine whether to base64 encode file data
    std::stringstream ef_data;
    if (is_file_ascii(ef_path))
    {
        // File is ASCII, simply read the buffer
        ef_data << ef_file.rdbuf();
    }
    else
    {
        // File is not ASCII, encode
        if (base64_encode(ef_path, ef_data))
        {
            fprintf(stderr, "%s Could not encode IF \"%s\"", VSFS_ERROR_PREFIX, ef_path.c_str());
            return EXIT_FAILURE;
        }
    }

    // Seek to the end of file to append any new records
    fs_file.seekg(0, std::ios::end);

    // Delete the record first, if existing
    bool existing = delete_record(fs_file, if_path);

    if (!existing)
    {
        // New record, ensure all subdirs are present
        size_t curr_delim;
        std::string curr_path = if_path;
        std::string inner_path;

        // Traverse from outermost intermediate directory, if any stage a dir is found, break, else create
        while ((curr_delim = curr_path.find_first_of(PATH_SEPARATOR)) != std::string::npos && curr_delim != 0)
        {
            inner_path += curr_path.substr(0, curr_delim + 1);
            if (!record_exists(inner_path, fs_file))
            {
                // Create intermediate directory
                fs_file << DIR_RECORD_IDENTIFIER << inner_path << '\n';
            }

            // Traverse inwards
            curr_path = curr_path.substr(curr_delim + 1);
        }
    }

    try
    {
        // Add new record entry to FS
        fs_file << FILE_RECORD_IDENTIFIER << if_path << '\n';

        // Write from EF to IF
        std::string ef_line;
        while (read_line(ef_data, ef_line))
        {
            // Truncating the line to 255 characters (' ' + content + '\n' or 1 + 253 + 1 = 255)
            size_t size = ef_line.size();

            // For lengths 254 (255 - 1 for the record type identifier ' ') and greater
            if (size >= MAXIMUM_RECORD_LENGTH - 1)
            {
                ef_line.resize(MAXIMUM_RECORD_LENGTH - 2);
                ef_line.append("\n");
            }
                // For lengths smaller than 254 and greater than 0
            else if (size > 0 && ef_line.at(size - 1) != '\n')
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
        fprintf(stderr, "%s Failed to write entry EF in FS %s\n",
            VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    // If FS was found zipped, re-zip it
    if (is_compressed)
        gzip_fs(true, fs_path);

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYIN_H
