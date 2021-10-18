#ifndef VSFS_RMDIR_H
#define VSFS_RMDIR_H

int vsfs_rmdir(int argc, char** argv) {
    // Verify command structure
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (!argv[3]) {
        fprintf(stderr, "%s No ID provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 4) {
        fprintf(stderr, "%s Arguments for command \"rmdir\", expected: 2, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2], id_path = argv[3];
    std::fstream fs_file;

    // Open FS file in read/write/append mode
    int err_code = EXIT_FAILURE;
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in | std::ios::out)) != EXIT_SUCCESS)
        return err_code;

    // Delete the ID
    if (!delete_record(fs_file, id_path, DIR_RECORD_IDENTIFIER)) {
        fprintf(stderr, "%s ID does not exist: %s\n", VSFS_ERROR_PREFIX, id_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_RMDIR_H
