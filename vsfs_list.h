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

// Used to calculate number of subdirs in a given dir
int calculate_subdir(dir* rootdir);

// Used to calculate the path from a file to a dir
std::string path_from(dir* from, file* to, bool inclusive);

int vsfs_list(int argc, char** argv) {
    if (!argv[2]) {
        fprintf(stderr, "%s No FS provided\n", VSFS_ERROR_PREFIX);
        return EINVAL;
    } else if (argc > 3) {
        fprintf(stderr, "%s Arguments for command \"list\", expected: 1, received: %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return E2BIG;
    }

    std::string fs_path = argv[2];
    std::stringstream fs_stream;
    try {
        std::fstream fs_file;
        int err_code = EXIT_FAILURE;

        // Open FS file in read mode
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in)) != EXIT_SUCCESS)
            return err_code;

        // Close the file resource since no write is done
        fs_stream << fs_file.rdbuf();
        fs_file.close();
    } catch (const std::ios_base::failure& failure) {
        fprintf(stderr, "%s Failed to read FS: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    std::vector<file*> fs_records;
    dir* fs_root = build_tree(fs_path, fs_stream, fs_records);

    // Retrieve and store FS attributes
    std::stringstream attr_stream;
    std::string fs_permissions, fs_owner_group, fs_datetime;
    struct stat fs_attr{};

    if (stat(fs_path.c_str(), &fs_attr) == EXIT_SUCCESS) {
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
    } else {
        fprintf(stderr, "%s FS does not exist: \"%s\"\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return ENOENT;
    }

    for (file* record : fs_records) {
        std::stringstream record_attr;
        bool is_dir = dynamic_cast<dir*>(record);
        record_attr << (is_dir ? 'd' : '-');
        record_attr << fs_permissions << ' ';
        record_attr
            << std::setfill(' ')
            << std::setw(3)
            << "1"
            << std::setw(0) << ' ';
        record_attr << fs_owner_group << ' ';
        record_attr << std::count(record->get_content().begin(), record->get_content().end(), '\n') << ' ';
        record_attr << fs_datetime << ' ';
        record_attr << record->get_path();

        printf("%s\n", record_attr.str().c_str());
        record_attr.str(std::string());
    }

    delete fs_root;
    return EXIT_SUCCESS;
}

bool is_dir(file* file) {
    return dynamic_cast<dir*>(file);
}

std::string path_from(dir* from, file* to, bool inclusive) {
    dir* curr_dir = to->get_parent();
    std::string path = to->get_name();
    while (curr_dir != from) {
        path.insert(0, curr_dir->get_name());
        if (!inclusive && curr_dir->get_parent() == from) {
            break;
        }
        curr_dir = curr_dir->get_parent();
    }

    return path;
}

int calculate_subdir(dir* rootdir) {
    int subdir_count = 0;
    for (file* file : rootdir->get_children()) {
        // If file is a subdir
        dir* subdir = dynamic_cast<dir*>(file);
        if (subdir)
            // Recursively enumerate subdirs of a subdir
            subdir_count += 1 + calculate_subdir(subdir);
    }

    return subdir_count;
}

#endif // VSFS_LIST_H
