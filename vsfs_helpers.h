#ifndef VSFS_HELPERS_H
#define VSFS_HELPERS_H

#include "dir.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

/*
 * Declarations
 */

// Verify whether the file exists
bool file_exists(const char* path);

// Open a file in the given path with the provided read/write/append modes
bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode);

// Open FS file in the given path with the specified mode
int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode);

// Open EF file in the given path with the specified mode
int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode, bool must_exist);

// Read a line with EOF checks
bool read_line(std::fstream& file, std::string& line);

// Write the FS records as in the dir
void write_fs(dir* root, std::fstream& fs_file);

// Write the records in the FS in order
void write_fs(const std::vector<file*>& records, std::fstream& fs_file);

// Delete the specified line from the file
void delete_line(std::fstream& fs_file, const std::string& fs_line);

// Delete the record to be written/appended to if exists
bool delete_record(std::fstream& fs_file, const std::string& record_name);

// Delete the directory and all its children recursively
void delete_dir(dir* root);

// Used to build the file-tree for the FS
dir* build_tree(
    const std::string& fs_path,
    std::fstream& fs_file,
    std::vector<file*>& fs_records,
    bool create_intermediate_dirs);

// Used to calculate number of subdirs in a given dir
int calculate_subdir(dir* rootdir);

/*
 * Definitions
 */

bool file_exists(const char* path)
{
    // Quickly check if the file exists
    struct stat attr{};
    return stat(path, &attr) == EXIT_SUCCESS;
}

bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode)
{
    // Allow throwing exceptions
    file.exceptions(file.exceptions() | std::ios::failbit | std::ios::badbit);

    // Open file in the given mode
    file.open(path, open_mode);

    return file.is_open();
}

int open_fs(const std::string& fs_path, std::fstream& fs_file, std::_Ios_Openmode open_mode)
{
    // Verify the FS exists
    if (!file_exists(fs_path.c_str()))
    {
        fprintf(stderr, "%s FS could not be found: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return EXIT_FAILURE;
    }

    // Check for the ".notes" extension
    std::string fs_extension;
    if (fs_path.find('.') == std::string::npos)
    {
        fprintf(stderr, "%s FS is missing extension: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return EXIT_FAILURE;
    }
    else if ((fs_extension = fs_path.substr(fs_path.find_last_of('.') + 1)) != FS_EXTENSION)
    {
        fprintf(stderr, "%s FS must end with the \".%s\" extension, unrecognised extension: %s\n",
            VSFS_ERROR_PREFIX, FS_EXTENSION, fs_extension.c_str());
        return EXIT_FAILURE;
    }

    try
    {
        // Open FS file in the given mode
        if (!open_file(fs_path, fs_file, open_mode))
        {
            fprintf(stderr, "%s FS could not be opened: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
            return EIO;
        }
    }
    catch (const std::ios::failure& failure)
    {
        fprintf(stderr, "%s FS I/O error: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    if (open_mode & std::ios::in)
    {
        // Verify first record
        std::string fs_line;
        read_line(fs_file, fs_line);
        if (fs_line != FS_FIRST_RECORD)
        {
            fprintf(stderr, "%s First record of FS must be \"%s\"\n", VSFS_ERROR_PREFIX, FS_FIRST_RECORD);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode, bool must_exist)
{
    // Verify the EF exists
    if (must_exist && !file_exists(ef_path.c_str()))
    {
        fprintf(stderr, "%s EF could not be found: %s\n", VSFS_ERROR_PREFIX, ef_path.c_str());
        return EXIT_FAILURE;
    }

    try
    {
        // Open EF file in the given mode
        if (!open_file(ef_path, ef_file, open_mode))
        {
            fprintf(stderr, "%s EF could not be opened: %s\n", VSFS_ERROR_PREFIX, ef_path.c_str());
            return EIO;
        }
    }
    catch (const std::ios::failure& failure)
    {
        fprintf(stderr, "%s EF I/O error: %s\n", VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    return EXIT_SUCCESS;
}

bool read_line(std::fstream& file, std::string& line)
{

    return !file.eof() && file.peek() != EOF && std::getline(file, line);
}

void write_fs(dir* root, std::fstream& fs_file)
{
    for (file* f: root->get_children())
    {
        dir* d = dynamic_cast<dir*>(f);
        if (!d)
        {
            fs_file << FILE_RECORD_IDENTIFIER << f->get_path() << '\n';
            std::stringstream content_stream(f->get_content());
            std::string to_be_written;
            while (std::getline(content_stream, to_be_written, '\n'))
                fs_file << ' ' << to_be_written << '\n';
        }
        else
        {
            fs_file << DIR_RECORD_IDENTIFIER << d->get_path() << '\n';
            write_fs(d, fs_file);
        }
    }
}

void write_fs(const std::vector<file*>& records, std::fstream& fs_file)
{
    fs_file << FS_FIRST_RECORD << '\n';
    for (file* f: records)
    {
        dir* d = dynamic_cast<dir*>(f);
        if (!d)
        {
            // If file is marked as deleted, put '#' instead
            fs_file
                << (f->is_deleted() ? DELETED_RECORD_IDENTIFIER : FILE_RECORD_IDENTIFIER)
                << f->get_path() << '\n';

            std::stringstream content_stream(f->get_content());
            std::string line;
            while (std::getline(content_stream, line, '\n'))
            {
                // Write record content line by line
                fs_file
                    << (f->is_deleted() ? DELETED_RECORD_IDENTIFIER : RECORD_CONTENT_IDENTIFIER)
                    << line << '\n';
            }
        }
        else
        {
            fs_file
                << (d->is_deleted() ? DELETED_RECORD_IDENTIFIER : DIR_RECORD_IDENTIFIER)
                << d->get_path() << '\n';
        }
    }
}

void delete_line(std::fstream& fs_file, const std::string& fs_line)
{
    // Save current write position
    auto curr_p = fs_file.tellp();

    // Seek to the beginning of the line and replace with '#'
    fs_file.seekp(std::ios::off_type(fs_file.tellp()) - (int) fs_line.size() - 1, std::ios_base::beg);
    fs_file.put(DELETED_RECORD_IDENTIFIER);

    // Reset write position
    fs_file.seekp(curr_p);
}

bool delete_record(std::fstream& fs_file, const std::string& record_name)
{
    bool deleted = false;
    std::string fs_line;

    // Move read position to the beginning
    fs_file.seekg(0, std::ios::beg);

    while (read_line(fs_file, fs_line) && !deleted)
    {
        // Record is found
        if (fs_line.front() == FILE_RECORD_IDENTIFIER && fs_line.substr(1) == record_name)
        {
            // Delete the record identifier
            delete_line(fs_file, fs_line);

            // Delete any additional content lines for file records
            while (read_line(fs_file, fs_line) && fs_line.front() == RECORD_CONTENT_IDENTIFIER)
                delete_line(fs_file, fs_line);

            deleted = true;
        }
    }

    // Restore read position
    fs_file.seekg(0, std::ios::beg);

    return deleted;
}

void delete_dir(dir* root)
{
    // Mark the parent dir as deleted
    root->set_deleted(true);
    for (file* f: root->get_children())
    {
        dir* d = dynamic_cast<dir*>(f);
        if (!d)
        {
            // Mark file records as deleted
            f->set_deleted(true);
        }
        else
        {
            // Recursively delete subdirs
            delete_dir(d);
        }
    }
}

dir* build_tree(
    const std::string& fs_path,
    std::fstream& fs_file,
    std::vector<file*>& fs_records,
    bool create_intermediate_dirs)
{
    // Root dir is the FS file itself
    dir* root = new dir(fs_path, fs_path);

    // The file being assessed currently
    file* curr_file = nullptr;

    // Read the contents of the given FS stream one line at a time
    std::string fs_line;
    while (read_line(fs_file, fs_line))
    {
        char record_symbol = fs_line.front();
        bool is_dir = record_symbol == DIR_RECORD_IDENTIFIER;
        std::string line_content = fs_line.substr(1);

        // If the record is a file ('@')/dir ('=')
        if (record_symbol == FILE_RECORD_IDENTIFIER || is_dir)
        {
            // The directory being assessed currently
            dir* curr_dir = root;
            std::string curr_path = line_content;

            // Index of the current '/' delimiter to keep track of path's depth level
            size_t curr_delim;

            // While additional intermediate subdirs exist, traverse down to the correct dir
            while ((curr_delim = curr_path.find_first_of(PATH_SEPARATOR)) != std::string::npos && curr_delim != 0)
            {
                // Extract subdir name and search for it in the current dir
                std::string subdir_name = curr_path.substr(0, curr_delim + 1);
                size_t found_index = curr_dir->find(subdir_name);
                dir* subdir;

                // If subdir does not exist
                if (found_index == curr_dir->get_children().size())
                {
                    // If an intermediate dir is not to be created, throw error
                    if (!is_dir && !create_intermediate_dirs)
                    {
                        fprintf(stderr, "%s FS dir \"%s\"could not be found for file %s\n",
                            VSFS_ERROR_PREFIX, subdir_name.c_str(), line_content.c_str());
                        return nullptr;
                    }
                    else
                    {
                        curr_dir->add_child((subdir = new dir(subdir_name, line_content)));
                    }
                }
                else
                {
                    // If subdir does exist, reuse
                    subdir = dynamic_cast<dir*>(curr_dir->get_children().at(found_index));
                }

                // Traverse down a level
                curr_dir = subdir;
                curr_path = curr_path.substr(curr_delim + 1);
            }

            // Insert the file/dir int the current dir
            if (!is_dir)
            {
                // If the record read is a file, place parent-child relationship
                file* f = new file(curr_path, line_content);
                curr_file = f;
                curr_dir->add_child(f);
                fs_records.push_back(curr_file);
            }
            else
            {
                // If instead the record is a dir, it is already added
                fs_records.push_back(curr_dir);
            }
        }
        else if (record_symbol == RECORD_CONTENT_IDENTIFIER && curr_file)
        {
            // Else append read content to the last assessed file
            curr_file->append_content(line_content + "\n");
        }
        else if (record_symbol == DELETED_RECORD_IDENTIFIER)
        {
            if (!curr_file || !curr_file->is_deleted())
            {
                // Keep deleted records as is, assume the first line is name, rest is content
                file* f = new file(line_content, line_content);
                f->set_deleted(true);
                curr_file = f;

                // Keep deleted records at root
                root->add_child(f);
                fs_records.push_back(curr_file);
            }
            else
            {
                curr_file->append_content(line_content + "\n");
            }
        }
    }

    return root;
}

void sort(dir* root)
{
    std::sort(root->get_children().begin(), root->get_children().end(),
        [](file* file1, file* file2)
        {
            dir* file1_dir = dynamic_cast<dir*>(file1);
            dir* file2_dir = dynamic_cast<dir*>(file2);

            // Dirs get higher privilege
            int file1_rank = file1_dir ? 0 : 1;
            int file2_rank = file2_dir ? 0 : 1;

            // Recursively sort subdirs
            if (file1_dir) sort(file1_dir);
            if (file2_dir) sort(file2_dir);

            // If both are dir/file compare lexicographically
            if (file1_rank == file2_rank)
                file1_rank += file2->get_name().compare(file1->get_name());

            return file1_rank > file2_rank;
        }
    );
}

bool is_internal_path_valid(const std::string& path)
{
    /*
     *  Not beginning with a '/'
     *  Not ending with a '/'
     *  Not '..'
     *  Not '.'
     */
    return (path.front() != PATH_SEPARATOR
        && (path.at(path.size() - 1) != PATH_SEPARATOR)
        && (path.size() > 1 && path.substr(0, 1) != "..")
        && path.front() != '.');
}

int calculate_subdir(dir* rootdir)
{
    int subdir_count = 0;
    for (file* file: rootdir->get_children())
    {
        // If file is a subdir
        dir* subdir = dynamic_cast<dir*>(file);
        if (subdir)
            // Recursively enumerate subdirs of a subdir
            subdir_count += 1 + calculate_subdir(subdir);
    }

    return subdir_count;
}

#endif // VSFS_HELPERS_H
