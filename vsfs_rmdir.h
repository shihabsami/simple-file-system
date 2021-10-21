#ifndef VSFS_RMDIR_H
#define VSFS_RMDIR_H

int vsfs_rmdir(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "%s Arguments for command \"rmdir\", expected: 2, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], id_path = argv[3];
    std::fstream fs_file;

    // Open the FS file in read mode
    int err_code = open_fs(fs_path, fs_file, std::ios::in);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Given ID name may not end with a '/' but the FS always has dirs ending with '/'
    size_t delim_position = id_path.find_first_of(PATH_SEPARATOR);
    if (delim_position == std::string::npos || id_path.at(id_path.size() - 1) != PATH_SEPARATOR)
    {
        // Append '/' if not present in the dir name
        id_path += PATH_SEPARATOR;
    }

    // Build the file tree to easily mark all children to be deleted
    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_file, fs_records, false);
    if (!fs_root)
        return EXIT_FAILURE;

    bool found = false;
    for (file* f: fs_records)
    {
        if (!f->is_deleted() && f->get_path() == id_path)
        {
            dir* d = dynamic_cast<dir*>(f);
            delete_dir(d);
            found = true;
            break;
        }
    }

    if (!found)
    {
        fprintf(stderr, "%s ID could not be found: %s\n",
            VSFS_ERROR_PREFIX, argv[3]);
        return EXIT_FAILURE;
    }

    // Reopen FS file in write mode with contents cleared
    fs_file.close();
    fs_file.clear();
    err_code = open_fs(fs_path, fs_file, std::ios::out | std::ios::trunc);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Write the FS with the ID and children marked as deleted
    write_fs(fs_records, fs_file);

    // Free memory
    delete fs_root;

    return EXIT_SUCCESS;
}

#endif // VSFS_RMDIR_H
