#ifndef VSFS_HELPERS_H
#define VSFS_HELPERS_H

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstring>


// Open FS file in the given path with the provided read/write/append modes
int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode);

// Open EF file in the given path with the provided read/write/append modes
int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode);

// Open EF file in the given path with the provided read/write/append modes
int open_if(const std::string& if_path, std::fstream& if_file, std::_Ios_Openmode open_mode);

void open_file(const std::string& path, std::fstream& stream, std::_Ios_Openmode open_mode);

int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode) {
    // Check for the ".notes" extension
    std::string fs_extension;
    if (fs_path.find('.') == std::string::npos) {
        fprintf(stderr, "FS must end with a \".notes\" extension\n");
        return FS_EXTENSION_ERROR;
    } else if ((fs_extension = fs_path.substr(fs_path.find_last_of('.') + 1)) != FS_EXTENSION) {
        fprintf(stderr, "Invalid FS extension %s\n", fs_extension.c_str());
        return FS_EXTENSION_ERROR;
    }

    try {
        // Open file in the given mode
        open_file(fs_path, fs_file, open_mode);
        return EXIT_SUCCESS;
    } catch (const std::ios_base::failure& failure) {
        fprintf(stderr, "FS I/O error: %s\n", failure.code().message().c_str());
        return failure.code().value();
    }
}

int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode) {
    try {
        open_file(ef_path, ef_file, open_mode);
        return EXIT_SUCCESS;
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "EF I/O error: %s\n", failure.code().message().c_str());
        return failure.code().value();
    }
}

int open_if(const std::string& if_path, std::fstream& if_file, std::_Ios_Openmode open_mode) {
    try {
        open_file(if_path, if_file, open_mode);
        return EXIT_SUCCESS;
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "IF I/O error: %s\n", failure.code().message().c_str());
        return failure.code().value();
    }
}

void open_file(const std::string& path, std::fstream& stream, std::_Ios_Openmode open_mode) {
    // Allow throwing exceptions
    stream.exceptions(stream.exceptions() | std::ios_base::failbit | std::ios_base::badbit);

    // Open file in the given mode
    stream.open(path, open_mode);
}

#endif // VSFS_HELPERS_H
