#ifndef VSFS_LIST_H
#define VSFS_LIST_H

#include "vsfs_constants.h"
#include "vsfs_helpers.h"

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#include <ctime>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

int vsfs_list(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 3)
    {
        fprintf(stderr, "%s Arguments for command \"list\", expected 1, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2];
    std::fstream fs_file;
    bool is_compressed{};

    // Open the FS file in read mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    // Build the filesystem tree
    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_file, fs_records, false);
    if (!fs_root)
        return EXIT_FAILURE;

    // Retrieve and store FS file's attributes
    std::stringstream attr_stream;
    std::string fs_permissions, fs_owner_group, fs_datetime;
    struct stat fs_attr{};
    stat(fs_path.c_str(), &fs_attr);

    // Permissions
    attr_stream << ((fs_attr.st_mode & S_IRUSR) ? 'r' : '-');
    attr_stream << ((fs_attr.st_mode & S_IWUSR) ? 'w' : '-');
    attr_stream << ((fs_attr.st_mode & S_IXUSR) ? 'x' : '-');
    attr_stream << ((fs_attr.st_mode & S_IRGRP) ? 'r' : '-');
    attr_stream << ((fs_attr.st_mode & S_IWGRP) ? 'w' : '-');
    attr_stream << ((fs_attr.st_mode & S_IXGRP) ? 'x' : '-');
    attr_stream << ((fs_attr.st_mode & S_IROTH) ? 'r' : '-');
    attr_stream << ((fs_attr.st_mode & S_IWOTH) ? 'w' : '-');
    attr_stream << ((fs_attr.st_mode & S_IXOTH) ? 'x' : '-');
    fs_permissions = attr_stream.str();
    attr_stream.str(std::string());

    // Owner
    struct passwd* pw;
    pw = getpwuid(fs_attr.st_uid);
    attr_stream << pw->pw_name << ' ';

    // Group
    struct group* gw;
    gw = getgrgid(fs_attr.st_gid);
    attr_stream << gw->gr_name;
    fs_owner_group = attr_stream.str();
    attr_stream.str(std::string());

    // Datetime
    std::time_t t(fs_attr.st_mtime);
    attr_stream << std::put_time(std::localtime(&t), "%b %d %H:%M:%S");
    fs_datetime = attr_stream.str();
    attr_stream.str(std::string());

    // Output records in the same order as they were read
    for (file* record: fs_records)
    {
        std::stringstream record_attr;
        dir* record_dir = dynamic_cast<dir*>(record);
        bool is_dir = record_dir != nullptr;

        // Send the attributes into the temporary stream to be able to create the string at once
        record_attr << (is_dir ? 'd' : '-');
        record_attr << fs_permissions << ' ';

        // Set the 3-character width for the number of links
        record_attr
            << std::setfill(' ')
            << std::setw(3)
            << (is_dir ? calculate_subdir(record_dir) : 1)
            << std::setw(0) << ' ';

        record_attr << fs_owner_group << ' ';

        // Size calculated as number of lines in the record's content
        record_attr << std::count(record->get_content().begin(),
            record->get_content().end(), '\n') << ' ';

        record_attr << fs_datetime << ' ';
        record_attr << record->get_path();

        // Finally, output the formatted string from the stream
        printf("%s\n", record_attr.str().c_str());
        record_attr.str(std::string());
    }

    // Free memory
    delete fs_root;

    // If FS was found zipped, re-zip it
    if (is_compressed)
        gzip_fs(true, fs_path);

    return EXIT_SUCCESS;
}

#endif // VSFS_LIST_H
