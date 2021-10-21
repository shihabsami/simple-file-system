#ifndef VSFS_LIST_H
#define VSFS_LIST_H

#include "vsfs_constants.h"
#include "vsfs_helpers.h"
#include "dir.h"

#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <pwd.h>
#include <grp.h>
#include <ftw.h>
#include <vector>
#include <sys/stat.h>

int vsfs_list(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 3)
    {
        fprintf(stderr, "%s Arguments for command \"list\", expected: 1, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2];
    std::fstream fs_file;

    try
    {
        // Open the FS file in read mode
        int err_code = EXIT_FAILURE;
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in)) != EXIT_SUCCESS)
            return err_code;
    }
    catch (const std::ios_base::failure& failure)
    {
        // If file could not be opened/read
        fprintf(stderr, "%s Failed to read FS: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    // Build the file tree
    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_file, fs_records, false);
    if (!fs_root)
        return EXIT_FAILURE;


    // Retrieve and store FS file's attributes
    std::stringstream attr_stream;
    std::string fs_permissions, fs_owner_group, fs_datetime;
    struct stat fs_attr{};

    if (stat(fs_path.c_str(), &fs_attr) == EXIT_SUCCESS)
    {
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

        // Owner/Group
        struct passwd* pw;
        pw = getpwuid(fs_attr.st_uid);
        attr_stream << pw->pw_name << ' ';

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
    }
    else
    {
        // If "stat()" failed to retrieve the file attributes i.e., file was not found
        fprintf(stderr, "%s FS does not exist: \"%s\"\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return ENOENT;
    }

    // Output records in the same order as they were read
    for (file* record: fs_records)
    {
        std::stringstream record_attr;
        bool is_dir = dynamic_cast<dir*>(record);

        // Send the attributes into the temporary stream to be able to create the string at once
        record_attr << (is_dir ? 'd' : '-');
        record_attr << fs_permissions << ' ';

        // Set the 3-character width for the number of links
        record_attr
            << std::setfill(' ')
            << std::setw(3)
            << "1"
            << std::setw(0) << ' ';

        record_attr << fs_owner_group << ' ';

        // Size calculated as number of lines in the record's content
        record_attr << std::count(record->get_content().begin(), record->get_content().end(), '\n') << ' ';

        record_attr << fs_datetime << ' ';
        record_attr << record->get_path();

        // Finally, output the formatted string from the stream
        printf("%s\n", record_attr.str().c_str());
        record_attr.str(std::string());
    }

    // Free memory
    delete fs_root;

    return EXIT_SUCCESS;
}

#endif // VSFS_LIST_H
