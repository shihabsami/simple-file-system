#ifndef VSFS_DEFRAG_H
#define VSFS_DEFRAG_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

int vsfs_defrag(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "%s Arguments for command \"defrag\", expected: 1, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2];
    std::fstream fs_file;

    // Open FS file in read mode
    int err_code = open_fs(fs_path, fs_file, std::ios::in);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Build a file tree to easily sort the records
    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_file, fs_records, true);
    sort(fs_root);

    // Reopen FS file in write mode with contents cleared
    err_code = open_fs(fs_path, fs_file, std::ios::out | std::ios::trunc);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Rewrite the new FS file
    fs_file << FS_FIRST_RECORD << '\n';
    write_fs(fs_root, fs_file);

    // Free memory
    delete fs_root;

    return EXIT_SUCCESS;
}

#endif // VSFS_DEFRAG_H
