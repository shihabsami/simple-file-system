#ifndef VSFS_COPYOUT_H
#define VSFS_COPYOUT_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

#include <fstream>
#include <sstream>

int vsfs_copyout(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "%s No IF provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[4]) {
        fprintf(stderr, "%s No EF provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 5) {
        fprintf(stderr, "%s Arguments for command \"copyout\", expected: 3, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2], if_path = argv[3], ef_path = argv[4];
    std::fstream fs_file, ef_file;

    // Open FS file in read/write/append mode
    int err_code = EXIT_FAILURE;
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
        return err_code;

    // Write from IF out to EF
    std::string fs_line;
    bool written = false;
    while (read_line(fs_file, fs_line) && !written) {
        if (fs_line.at(0) == FILE_RECORD_IDENTIFIER) {
            if (fs_line.substr(1) == if_path) {
                // Open EF file in write mode, create file if file does not exist, if exists clear the existing content
                if ((err_code = open_ef(ef_path, ef_file, std::ios::out | std::ios::trunc)) != EXIT_SUCCESS)
                    return err_code;
                while (read_line(fs_file, fs_line) && fs_line.at(0) == RECORD_CONTENT_IDENTIFIER)
                    ef_file << fs_line.substr(1) << '\n';

                written = true;
            }
        }
    }

    if (!written) {
        fprintf(stderr, "%s No record found for IF in FS\n", VSFS_ERROR_PREFIX);
        return ENOENT;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYOUT_H
