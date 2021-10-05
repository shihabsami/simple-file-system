#ifndef VSFS_COPYIN_H
#define VSFS_COPYIN_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

#include <fstream>
#include <sstream>

int vsfs_copyin(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "No FS provided\n");
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "No EF provided\n");
        return EINVAL;
    } else if (!argv[4]) {
        fprintf(stderr, "No IF provided\n");
        return EINVAL;
    } else if (argc > 5) {
        fprintf(stderr, "Arguments for command \"list\", expected: 4, received: %d\n", argc - 1);
        return E2BIG;
    }

    std::string fs_path = argv[2], ef_path = argv[3], if_path = argv[4];
    std::fstream fs_file, ef_file, if_file;

    try {
        int err_code = EXIT_FAILURE;
        // Open FS file in read/write/append mode
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS || !fs_file.is_open())
            return err_code;

        // Verify first record
        std::string fs_line;
        std::getline(fs_file, fs_line);
        if (fs_line != FS_FIRST_RECORD) {
            fprintf(stderr, "First record of FS must be \"NOTES V1.0\"\n");
            return FS_FIRST_RECORD_ERROR;
        }

        // Open EF file in read mode, throws error if file does not exist
        if ((err_code = open_ef(ef_path, ef_file, std::ios::in)) != EXIT_SUCCESS || !ef_file.is_open())
            return err_code;

        // Open IF file in read mode, create file if file does not exist, if exists clear the existing content
        if ((err_code = open_if(if_path, if_file, std::ios::out | std::ios::trunc)) != EXIT_SUCCESS || !if_file.is_open())
            return err_code;

        // Delete the record to be written/appended to
        while (!fs_file.eof() && fs_file.peek() != EOF && std::getline(fs_file, fs_line)) {
            if (fs_line.at(0) == FILE_RECORD_IDENTIFIER) {
                if (fs_line.substr(1) == if_path) {
                    fs_file.seekp(std::ios::off_type(fs_file.tellp()) - (int) fs_line.size() - 1,
                        std::ios_base::beg);
                    fs_file.put(DELETED_RECORD_IDENTIFIER);
                }
            }
        }

        // Verify if last char is a newline
        fs_file.seekg(-1, std::ios::end);
        char last_char;
        fs_file.get(last_char);

        // Close and reopen FS in append mode
        fs_file.close();
        fs_file.clear();
        fs_file.open(fs_path, std::ios::app);

        // Add new record to FS
        if (last_char != '\n')
            fs_file << '\n';
        fs_file << FILE_RECORD_IDENTIFIER << if_path << '\n';

        // Write from EF to IF
        std::string ef_line;
        while (!ef_file.eof() && ef_file.peek() != EOF && std::getline(ef_file, ef_line)) {
            size_t size = ef_line.size();
            if (size > MAXIMUM_RECORD_LENGTH) {
                ef_line.resize(MAXIMUM_RECORD_LENGTH);
                ef_line.append("\n");
            } else if ((size == MAXIMUM_RECORD_LENGTH) || (size > 0 && ef_line.at(size - 1) != '\n')) {
                ef_line.append("\n");
            }

            if_file << ef_line;
            fs_file << RECORD_CONTENT_IDENTIFIER << ef_line;
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYIN_H
