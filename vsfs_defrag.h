#ifndef VSFS_DEFRAG_H
#define VSFS_DEFRAG_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

void write_dir(dir* d, std::fstream& fs_file);

int vsfs_defrag(int argc, char** argv) {
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 3) {
        fprintf(stderr, "%s Arguments for command \"defrag\", expected: 1, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2];
    std::fstream fs_file;
    std::stringstream fs_stream;
    int err_code = EXIT_FAILURE;

    // Open FS file in read mode
    if ((err_code = open_fs(fs_path, fs_file, std::ios::in)) != EXIT_SUCCESS)
        return err_code;

    // Close the file resource since no write is done
    fs_stream << fs_file.rdbuf();
    fs_file.close();

    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_stream, fs_records);
    sort(fs_root);

    // Open FS file in write mode, clearing existing content
    if ((err_code = open_fs(fs_path, fs_file, std::ios::out | std::ios::trunc)) != EXIT_SUCCESS)
        return err_code;

    fs_file << FS_FIRST_RECORD << '\n';
    write_dir(fs_root, fs_file);

    return EXIT_SUCCESS;
}

void write_dir(dir* d, std::fstream& fs_file) {
    for (file* f : d->get_children()) {
        dir* fd = dynamic_cast<dir*>(f);
        if (!fd) {
            fs_file << FILE_RECORD_IDENTIFIER << f->get_path() << '\n';
            std::stringstream content_stream(f->get_content());
            std::string to_be_written;
            while (std::getline(content_stream, to_be_written, '\n'))
                fs_file << ' ' << to_be_written << '\n';
        } else {
            fs_file << DIR_RECORD_IDENTIFIER << fd->get_path() << '\n';
            write_dir(fd, fs_file);
        }
    }
}

#endif // VSFS_DEFRAG_H
