#ifndef VSFS_LIST_H
#define VSFS_LIST_H

#include "vsfs_constants.h"
#include "vsfs_helpers.h"

#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <pwd.h>
#include <grp.h>
#include <ftw.h>
#include <sys/stat.h>

// Used in conjunction with the stat() function to obtain file attributes
std::string record_stat(const std::string& record_path, const std::fstream& fs_file);

// TODO "Number of subdirs" only within the VSFS or in the external file system ???
//// Used in conjunction with the "ftw()" as a callback function
//int file_tree_walk(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf);
//
//// Used to calculate number of subdirs in a given dir/path
//int num_subdir = 0;
//int calculate_subdir(const char* dirpath);

// Used to calculate number of subdirs in a given dir/path
int calculate_subdir(const std::string& dirpath, const std::fstream& fs_file);

int vsfs_list(int argc, char** argv) {
    if (!argv[2]) {
        fprintf(stderr, "No FS provided\n");
        return EINVAL;
    } else if (argc > 3) {
        fprintf(stderr, "Arguments for command \"list\", expected: 2, received: %d\n", argc - 1);
        return E2BIG;
    }

    try {
        std::string fs_path = argv[2];
        std::fstream fs_file;
        int err_code;
        // Open FS file in read mode
        if ((err_code = open_fs(fs_path, fs_file, std::ios::in)) != EXIT_SUCCESS)
            return err_code;

        // Verify first record
        std::string fs_line;
        std::getline(fs_file, fs_line);
        if (fs_line != FS_FIRST_RECORD) {
            fprintf(stderr, "First record of FS must be \"NOTES V1.0\"\n");
            return FS_FIRST_RECORD_ERROR;
        }

        while (std::getline(fs_file, fs_line)) {
            if (fs_line.at(0) == '@' || fs_line.at(0) == '=') {
                std::string record_path = fs_line.substr(1);
                std::string stat = record_stat(record_path, fs_file);
                if (!stat.empty()) {
                    printf("%s\n", stat.c_str());
                } else {
                    fprintf(stderr, "Record %s does not exist in FS\n", record_path.c_str());
                    return ENOENT;
                }
            }
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

std::string record_stat(const std::string& record_path, const std::fstream& fs_file) {
    std::stringstream sstream;
    struct stat file_attrib{};

    if (stat(record_path.c_str(), &file_attrib) == 0) {
        // Permissions
        sstream << ((S_ISDIR(file_attrib.st_mode)) ? 'd' : '-');
        sstream << ((file_attrib.st_mode & S_IRUSR) ? 'r' : '-');
        sstream << ((file_attrib.st_mode & S_IWUSR) ? 'w' : '-');
        sstream << ((file_attrib.st_mode & S_IXUSR) ? 'x' : '-');
        sstream << ((file_attrib.st_mode & S_IRGRP) ? 'r' : '-');
        sstream << ((file_attrib.st_mode & S_IWGRP) ? 'w' : '-');
        sstream << ((file_attrib.st_mode & S_IXGRP) ? 'x' : '-');
        sstream << ((file_attrib.st_mode & S_IROTH) ? 'r' : '-');
        sstream << ((file_attrib.st_mode & S_IWOTH) ? 'w' : '-');
        sstream << ((file_attrib.st_mode & S_IXOTH) ? 'x' : '-');

        // Number of hard inks/subdirs
        std::stringstream temp;
        sstream << ' ';
        sstream << std::setfill(' ') << std::setw(3) << (S_ISDIR(file_attrib.st_mode)
            ? calculate_subdir(record_path, fs_file) : file_attrib.st_nlink);

        // Owner
        sstream << ' ';
        struct passwd* pw;
        pw = getpwuid(file_attrib.st_uid);
        temp << file_attrib.st_uid << '+' << pw->pw_name;
        sstream << std::setfill(' ') << std::setw(16) << temp.str();
        temp.str(std::string());

        // Group
        sstream << ' ';
        struct group* gw;
        gw = getgrgid(file_attrib.st_gid);
        temp << file_attrib.st_gid << '+' << gw->gr_name;
        sstream << std::setfill(' ') << std::setw(16) << temp.str();
        temp.str(std::string());

        // Size
        sstream << ' ';
        sstream << std::setfill(' ') << std::setw(8) << file_attrib.st_size;

        // Datetime
        sstream << ' ';
        std::time_t t(file_attrib.st_mtime);
        sstream << std::put_time(std::localtime(&t), "%b %d %H:%M %Y");

        // Path
        sstream << ' ';
        sstream << std::setw(0) << record_path;
    }

    return sstream.str();
}

//int file_tree_walk(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
//    if (typeflag == FTW_D)
//        num_subdir++;
//
//    return EXIT_SUCCESS;
//}
//
//int calculate_subdir(const char* dirpath) {
//    num_subdir = 0;
//    ftw(dirpath, reinterpret_cast<__ftw_func_t>(file_tree_walk), 1);
//    return num_subdir;
//}

int calculate_subdir(const std::string& dirpath, const std::fstream & fs_file) {
    // TODO Calculate subdir that only exist within the VSFS
    return 0;
}

#endif // VSFS_LIST_H
