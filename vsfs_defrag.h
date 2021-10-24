#ifndef VSFS_DEFRAG_H
#define VSFS_DEFRAG_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

int vsfs_defrag(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 3)
    {
        fprintf(stderr, "%s Arguments for command \"defrag\", expected 1, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2];
    std::fstream fs_file;
    bool is_compressed{};

    // Open FS file in read mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Build a file tree to easily sort the records
    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_file, fs_records, false);
    sort(fs_root);

    // Close and reopen FS file in write mode with contents cleared
    fs_file.clear();
    fs_file.close();
    err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::out | std::ios::trunc);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Rewrite the new FS file
    fs_file << FS_FIRST_RECORD << '\n';
    write_fs(fs_root, fs_file);

    // Free memory
    delete fs_root;

    // If FS was found zipped, re-zip it
    if (is_compressed)
        gzip_fs(true, fs_path);

    return EXIT_SUCCESS;
}

#endif // VSFS_DEFRAG_H
