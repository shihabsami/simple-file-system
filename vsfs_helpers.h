#ifndef VSFS_HELPERS_H
#define VSFS_HELPERS_H

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstring>
#include "dir.h"

// Open FS file in the given path with the specified mode
int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode);

// Open EF file in the given path with the specified mode
int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode);

// Open a file in the given path with the provided read/write/append modes
bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode);

// Read a line with EOF checks
bool read_line(std::fstream& file, std::string& line);

// Delete the specified line from the file
void delete_line(std::fstream& fs_file, const std::string& fs_line);

// Delete the record to be written/appended to if exists
bool delete_record(std::fstream& fs_file, const std::string& record_name, char record_type);

// Used to build the file-tree for the FS
dir* build_tree(const std::string& fs_path, std::stringstream& fs_stream, std::vector<file*>& fs_records);

int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode) {
    // Check for the ".notes" extension
    std::string fs_extension;
    if (fs_path.find('.') == std::string::npos) {
        fprintf(stderr, "%s FS is missing extension: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return EXIT_FAILURE;
    } else if ((fs_extension = fs_path.substr(fs_path.find_last_of('.') + 1)) != FS_EXTENSION) {
        fprintf(stderr, "%s FS must end with the \".%s\" extension, unrecognised extension: %s\n",
            VSFS_ERROR_PREFIX, FS_EXTENSION, fs_extension.c_str());
        return EXIT_FAILURE;
    }

    try {
        // Open FS file in the given mode
        if (!open_file(fs_path, fs_file, open_mode)) {
            fprintf(stderr, "%s FS could not be opened: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
            return EIO;
        }
    } catch (const std::ios::failure& failure) {
        fprintf(stderr, "%s FS I/O error: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    if (open_mode & std::ios::in) {
        // Verify first record
        std::string fs_line;
        read_line(fs_file, fs_line);
        if (fs_line != FS_FIRST_RECORD) {
            fprintf(stderr, "%s First record of FS must be \"%s\"\n", VSFS_ERROR_PREFIX, FS_FIRST_RECORD);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode) {
    try {
        // Open EF file in the given mode
        if (open_file(ef_path, ef_file, open_mode))
            return EXIT_SUCCESS;
        else
            return EIO;
    } catch (const std::ios::failure& failure) {
        fprintf(stderr, "%s EF could not be opened: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }
}

bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode) {
    // Allow throwing exceptions except in write mode
    file.exceptions(file.exceptions() | std::ios::failbit | std::ios::badbit);
    // Open file in the given mode
    file.open(path, open_mode);
    return file.is_open();
}

bool read_line(std::fstream& file, std::string& line) {
    return !file.eof() && file.peek() != EOF && std::getline(file, line);
}

void delete_line(std::fstream& fs_file, const std::string& fs_line) {
    auto curr_p = fs_file.tellp();
    fs_file.seekp(std::ios::off_type(fs_file.tellp()) - (int) fs_line.size() - 1, std::ios_base::beg);
    fs_file.put(DELETED_RECORD_IDENTIFIER);
    fs_file.seekp(curr_p);
}

bool delete_record(std::fstream& fs_file, const std::string& record_name, char record_type) {
    bool deleted = false;
    std::string fs_line;

    // Move read position to the beginning
    fs_file.seekg(0, std::ios::beg);
    while (read_line(fs_file, fs_line)) {
        if (fs_line.at(0) == record_type) {
            if (fs_line.substr(1) == record_name) {
                // Delete the record identifier
                delete_line(fs_file, fs_line);
                // Delete any additional content lines for file records
                if (record_type == FILE_RECORD_IDENTIFIER)
                    while (read_line(fs_file, fs_line) && fs_line.at(0) == RECORD_CONTENT_IDENTIFIER)
                        delete_line(fs_file, fs_line);

                deleted = true;
            }
        }
    }
    // Restore read position
    fs_file.seekg(0, std::ios::beg);

    return deleted;
}

dir* build_tree(const std::string& fs_path, std::stringstream& fs_stream, std::vector<file*>& fs_records) {
    dir* root = new dir(fs_path, fs_path);
    std::stringstream fs_str(fs_stream.str());
    std::string fs_line;
    file* curr_file = nullptr;
    while (std::getline(fs_str, fs_line)) {
        bool is_dir = false;
        char record_symbol = fs_line.at(0);
        std::string line_content = fs_line.substr(1);

        if (record_symbol == FILE_RECORD_IDENTIFIER || (is_dir = (record_symbol == DIR_RECORD_IDENTIFIER))) {
            // Current directory at the initial level
            dir* curr_dir = root;
            std::string curr_path = line_content;
            // Index of the current '/' delimiter to keep track of dir level
            size_t curr_delim;

            // While additional intermediate subdirs exist
            while ((curr_delim = curr_path.find_first_of('/')) != std::string::npos && curr_delim != 0) {
                // Verify if subdir exists, create if not
                std::string subdir_name = curr_path.substr(0, curr_delim + 1);
                size_t index = curr_dir->find(subdir_name);
                dir* subdir;
                if (index == curr_dir->get_children().size())
                    curr_dir->add_child((subdir = new dir(subdir_name, line_content)));
                else
                    subdir = dynamic_cast<dir*>(curr_dir->get_children().at(index));

                // Traverse down a level
                curr_dir = subdir;
                curr_path = curr_path.substr(curr_delim + 1);
            }

            if (!is_dir) {
                file* f = new file(curr_path, line_content);
                curr_file = f;
                curr_dir->add_child(f);
                fs_records.push_back(curr_file);
            } else {
                fs_records.push_back(curr_dir);
            }
        } else if (record_symbol == RECORD_CONTENT_IDENTIFIER && curr_file) {
            size_t size = line_content.size();
            if (size > MAXIMUM_RECORD_LENGTH) {
                line_content.resize(MAXIMUM_RECORD_LENGTH - 1);
                line_content.append("\n");
            } else if (size > 0 && fs_line.at(size - 1) != '\n') {
                line_content.append("\n");
            }
            curr_file->append_content(line_content);
        }
    }

    return root;
}

#include <algorithm>

void sort(dir* root) {
    std::sort(root->get_children().begin(), root->get_children().end(),
        [](file* file1, file* file2){
        // Dirs get higher privilege
        dir* file1_dir = dynamic_cast<dir*>(file1);
        dir* file2_dir = dynamic_cast<dir*>(file2);

        int file1_rank = file1_dir ? 0 : 1;
        int file2_rank = file2_dir ? 0 : 1;

        if (file1_dir)
            sort(file1_dir);
        if (file2_dir)
            sort(file2_dir);

        // If none are both dir/file compare lexicographically
        if (file1_rank == file2_rank)
            file1_rank += file2->get_name().compare(file1->get_name());
        return file1_rank > file2_rank;
    });
}

#endif // VSFS_HELPERS_H
