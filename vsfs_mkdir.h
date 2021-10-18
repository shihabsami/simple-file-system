#ifndef VSFS_MKDIR_H
#define VSFS_MKDIR_H

#include "vsfs_helpers.h"

int vsfs_mkdir(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "%s No ID provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    }

    std::string fs_path = argv[2], dir_name = argv[3];
    std::fstream fs_file;
    int err_code = EXIT_FAILURE;
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::app)) != EXIT_SUCCESS)
        return err_code;

    std::string fs_line;
    while (read_line(fs_file, fs_line)) {
        if (fs_line.at(0) == DIR_RECORD_IDENTIFIER) {
            if (fs_line.substr(1) == dir_name) {
                fprintf(stderr, "%s ID already exists\n", VSFS_ERROR_PREFIX);
                return EXIT_FAILURE;
            }
        }
    }

    fs_file.clear();
    try {
        fs_file << DIR_RECORD_IDENTIFIER << dir_name << '\n';
    } catch (std::ios::failure& failure) {
        fprintf(stderr, "%s", failure.code().message().c_str());
        return EIO;
    }
    return EXIT_SUCCESS;
}

#endif // VSFS_MKDIR_H
