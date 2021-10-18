#ifndef VSFS_COPYIN_H
#define VSFS_COPYIN_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

#include <fstream>
#include <sstream>

bool if_valid(const std::string& if_path) {
    return (if_path.at(0) == '/'
        || if_path.at(if_path.at(if_path.size() - 1) == '/')
//        || (if_path.size() > 1 && if_path.substr(0, 1) == "..")
        || if_path == ".."
//        || if_path.at(0) == '.'
        || if_path == ".");
}

int vsfs_copyin(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "%s No EF provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[4]) {
        fprintf(stderr, "%s No IF provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 5) {
        fprintf(stderr, "%s  Arguments for command \"copyin\", expected: 3, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2], ef_path = argv[3], if_path = argv[4];
    std::fstream fs_file, ef_file;

    try {
        int err_code = EXIT_FAILURE;
        // Open FS file in read/write/append mode
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
            return err_code;

        // Open EF file in read mode, throws error if file does not exist
        if ((err_code = open_ef(ef_path, ef_file, std::ios::in)) != EXIT_SUCCESS)
            return err_code;

        // Check whether IF is valid
        if (!if_valid(if_path)) {
            fprintf(stderr, "%s IF is invalid: %s\n",
                VSFS_ERROR_PREFIX, if_path.c_str());
            return EXIT_FAILURE;
        }

        // Delete the record first, if exists
        delete_record(fs_file, if_path, FILE_RECORD_IDENTIFIER);

        // Verify if last char is a newline
        fs_file.seekg(-1, std::ios::end);
        char last_char;
        fs_file.get(last_char);

        // Close and reopen FS in append mode
        fs_file.close();
        fs_file.clear();
        fs_file.open(fs_path, std::ios::app);
        if (last_char != '\n')
            fs_file << '\n';

        // Add new record to FS
        fs_file << FILE_RECORD_IDENTIFIER << if_path << '\n';
        std::string ef_line;
        // Write from EF to IF
        while (!ef_file.eof() && ef_file.peek() != EOF && std::getline(ef_file, ef_line)) {
            size_t size = ef_line.size();
            if (size > MAXIMUM_RECORD_LENGTH) {
                ef_line.resize(MAXIMUM_RECORD_LENGTH);
                ef_line.append("\n");
            } else if ((size == MAXIMUM_RECORD_LENGTH) || (size > 0 && ef_line.at(size - 1) != '\n')) {
                ef_line.append("\n");
            }

            fs_file << RECORD_CONTENT_IDENTIFIER << ef_line;
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "%s Unknown I/O error: %s\n",
            VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYIN_H
