#ifndef VSFS_RM_H
#define VSFS_RM_H

#include "vsfs_helpers.h"

int vsfs_rm(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "%s No IF provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 4) {
        fprintf(stderr, "%s Arguments for command \"rm\", expected: 2, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2], if_path = argv[3];
    std::fstream fs_file;

    // Open FS file in read/write/append mode
    int err_code = EXIT_FAILURE;
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
        return err_code;

    // Delete the IF
    if (!delete_record(fs_file, if_path, FILE_RECORD_IDENTIFIER)) {
        fprintf(stderr, "%s IF does not exist: %s\n", VSFS_ERROR_PREFIX, if_path.c_str());
        return EXIT_FAILURE;
    }

    printf("IF deleted successfully\n");
    return EXIT_SUCCESS;
}

#endif // VSFS_RM_H
