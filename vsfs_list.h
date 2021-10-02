#ifndef VSFS_LIST_H
#define VSFS_LIST_H

#include "vsfs_commands.h"

#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <pwd.h>
#include <grp.h>
#include <ftw.h>
#include <sys/stat.h>

// Used in conjunction with the "stat()" function to obtain file attributes
std::string file_stat(const char* fpath);

// Used in conjunction with the "ftw()" as a callback function
int file_tree_walk(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf);

// Used to calculate number of subdirs in a given dir/path
int num_subdir = 0;
int calculate_subdir(const char* dirpath);

int vsfs_list(int argc, char** argv) {
    int exit_code = EXIT_FAILURE + LIST;
    if (!argv[2]) {
        fprintf(stderr, "No FS provided\n");
        return exit_code;
    } else if (argc > 3) {
        fprintf(stderr, "Arguments for command \"list\", expected: 2, received: %d\n", argc - 1);
        return exit_code;
    }

    try {
        std::string fs_path = argv[2];
        std::string extension;
        if (fs_path.find('.') == std::string::npos) {
            fprintf(stderr, "FS must end with \".notes\" extension\n");
            return exit_code;
        } else if ((extension = fs_path.substr(fs_path.find_last_of('.') + 1)) != "notes") {
            fprintf(stderr, "Invalid FS extension %s\n", extension.c_str());
            return exit_code;
        }

        std::ifstream file;
        file.exceptions(file.exceptions() | std::ifstream::failbit | std::ifstream::badbit);
        file.open(argv[2]);

        if (file.peek() == EOF) {
            fprintf(stderr, "FS is empty\n");
            return exit_code;
        }

        std::stringstream sstream;
        sstream << file.rdbuf();
        file.close();

        std::string line;
        std::getline(sstream, line);
        if (line != "NOTES V1.0") {
            fprintf(stderr, "FS must begin with \"NOTES V1.0\"\n");
            return exit_code;
        }

        while (std::getline(sstream, line)) {
            if (line.at(0) == '@' || line.at(0) == '=') {
                std::string record_path = line.substr(1);
                std::string stat = file_stat(record_path.c_str());
                if (!stat.empty()) {
                    printf("%s\n", stat.c_str());
                } else {
                    fprintf(stderr, "Record %s does not exist in FS\n", record_path.c_str());
                    return exit_code;
                }
            }
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.what());
        return exit_code;
    }

    return EXIT_SUCCESS;
}

std::string file_stat(const char* path) {
    std::stringstream sstream;
    struct stat file_attrib{};

    if (stat(path, &file_attrib) == 0) {
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

        struct passwd* pw;
        pw = getpwuid(file_attrib.st_uid);
        struct group* gw;
        gw = getgrgid(file_attrib.st_gid);

        std::stringstream temp;
        sstream << ' ';
        sstream << std::setfill(' ') << std::setw(3) << (S_ISDIR(file_attrib.st_mode)
            ? calculate_subdir(path) : file_attrib.st_nlink);

        sstream << ' ';
        temp << file_attrib.st_uid << '+' << pw->pw_name;
        sstream << std::setfill(' ') << std::setw(16) << temp.str();
        temp.str(std::string());

        sstream << ' ';
        temp << file_attrib.st_gid << '+' << gw->gr_name;
        sstream << std::setfill(' ') << std::setw(16) << temp.str();
        temp.str(std::string());

        sstream << ' ';
        sstream << std::setfill(' ') << std::setw(8) << file_attrib.st_size;
        sstream << ' ';
        std::time_t t(file_attrib.st_mtime);
        sstream << std::put_time(std::localtime(&t), "%b %d %H:%M %Y");
        sstream << ' ';
        sstream << std::setw(0) << path;
    }

    return sstream.str();
}

int file_tree_walk(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    if (typeflag == FTW_D)
        num_subdir++;

    return EXIT_SUCCESS;
}

int calculate_subdir(const char* dirpath) {
    num_subdir = 0;
    ftw(dirpath, reinterpret_cast<__ftw_func_t>(file_tree_walk), 16);
    return num_subdir;
}

#endif // VSFS_LIST_H
