#ifndef VSFS_HELPERS_H
#define VSFS_HELPERS_H

#include "dir.h"
#include "vsfs_externals.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

/*
 * Contains most of the utility helper functions that are shared across different VSFS commands.
 */

/*
 * Declarations
 */

// Verify whether the file exists
bool file_exists(const char* path);

// Verify whether a record exists
bool record_exists(const std::string& record, std::fstream& fs_file);

// Open a file in the given path with the provided read/write/append modes
bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode);

// Open FS file in the given path with the specified mode
int open_fs(std::string& fs_path, std::fstream& fs_file, bool& is_compressed, std::_Ios_Openmode open_mode);

// Open EF file in the given path with the specified mode
int open_ef(const std::string& ef_path, std::fstream& ef_file, std::_Ios_Openmode open_mode, bool must_exist);

// Read a line with EOF checks
bool read_line(std::iostream& file, std::string& line);

// Write the FS records recursively starting at root
void write_fs(dir* root, std::fstream& fs_file);

// Delete the specified line from the file
void delete_line(std::fstream& fs_file, const std::string& fs_line);

// Delete the specified record from the file
bool delete_record(std::fstream& fs_file, const std::string& record_name);

// Delete the specified directory and all its children from the file
bool delete_dir(std::fstream& fs_file, const std::string& dir_name);

/*
 * Build the filesystem tree data structure from the FS.
 *
 * fs_path - The location for the FS.
 * fs_file - The fstream reference to be opened.
 * fs_records - A vector reference to store pointers to records in order of read.
 * create_intermediate_dirs - Whether the algorithm should create intermediate dirs.
 **/
dir* build_tree(
    const std::string& fs_path,
    std::fstream& fs_file,
    std::vector<file*>& fs_records,
    bool create_intermediate_dirs);

// Used to calculate number of subdirs in a given dir
int calculate_subdir(dir* rootdir);

// Check whether the given internal path is valid
bool is_internal_path_valid(const std::string& path, bool is_dir);

/*
 * Definitions
 */

bool file_exists(const char* path)
{
    // Quickly check if the file exists
    struct stat attr{};
    return stat(path, &attr) == EXIT_SUCCESS;
}

bool record_exists(const std::string& record, std::fstream& fs_file)
{
    // Save current read position
    auto curr_g = fs_file.tellg();

    // Seek to the beginning of the file
    fs_file.seekg(0, std::ios::beg);

    bool exists{};
    std::string fs_line;
    while (read_line(fs_file, fs_line) && !exists)
    {
        // Record is found
        if ((fs_line.front() == FILE_RECORD_IDENTIFIER || fs_line.front() == DIR_RECORD_IDENTIFIER)
            && fs_line.substr(1) == record)
        {
            exists = true;
        }
    }

    // Restore read position
    fs_file.seekg(curr_g);

    return exists;
}

bool open_file(const std::string& path, std::fstream& file, std::_Ios_Openmode open_mode)
{
    // Allow throwing exceptions
    file.exceptions(file.exceptions() | std::ios::failbit | std::ios::badbit);

    // Open file in the given mode
    file.open(path, open_mode);

    return file.is_open();
}

int open_fs(std::string& fs_path, std::fstream& fs_file, bool& is_compressed, std::_Ios_Openmode open_mode)
{
    // Verify the FS exists
    if (!file_exists(fs_path.c_str()))
    {
        fprintf(stderr, "%s FS could not be found %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return ENOENT;
    }

    // Check for the ".notes" or ".gz" extension
    std::string fs_extension;
    if (fs_path.find('.') == std::string::npos)
    {
        fprintf(stderr, "%s FS is missing extension %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());
        return EXIT_FAILURE;
    }

    fs_extension = fs_path.substr(fs_path.find_last_of('.') + 1);
    if (fs_extension != FS_EXTENSION && fs_extension != GZ_EXTENSION)
    {
        fprintf(stderr, "%s FS must end with the \".%s\" or \".%s\" extension, unrecognised extension: %s\n",
            VSFS_ERROR_PREFIX, FS_EXTENSION, GZ_EXTENSION, fs_extension.c_str());
        return EXIT_FAILURE;
    }

    // If file is zipped, unzip using gzip
    is_compressed = fs_extension == GZ_EXTENSION;
    if (is_compressed)
    {
        gzip_fs(false, fs_path);
    }

    try
    {
        // Open FS file in the given mode
        if (!open_file(fs_path, fs_file, open_mode))
        {
            fprintf(stderr, "%s FS could not be opened: %s\n",
                VSFS_ERROR_PREFIX, fs_path.c_str());
            return EIO;
        }
    }
    catch (const std::ios::failure& failure)
    {
        fprintf(stderr, "%s FS I/O error: %s\n",
            VSFS_ERROR_PREFIX, failure.code().message().c_str());
        return failure.code().value();
    }

    // If opened in read mode
    if (open_mode & std::ios::in)
    {
        // Verify first record
        std::string fs_line;
        read_line(fs_file, fs_line);
        if (fs_line != FS_FIRST_RECORD)
        {
            fprintf(stderr, "%s First record of FS must be \"%s\"\n",
                VSFS_ERROR_PREFIX, FS_FIRST_RECORD);
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
        return ENOENT;
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

bool read_line(std::iostream& file, std::string& line)
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
            // If record is a file
            fs_file << FILE_RECORD_IDENTIFIER << f->get_path() << '\n';
            std::stringstream content_stream(f->get_content());
            std::string to_be_written;

            // Write record's content
            while (std::getline(content_stream, to_be_written, '\n'))
                fs_file << RECORD_CONTENT_IDENTIFIER << to_be_written << '\n';
        }
        else
        {
            // If record is a dir, recursively write all children
            fs_file << DIR_RECORD_IDENTIFIER << d->get_path() << '\n';
            write_fs(d, fs_file);
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

    // Restore write position
    fs_file.seekp(curr_p);
}

bool delete_record(std::fstream& fs_file, const std::string& record_name)
{
    bool deleted{};
    std::string fs_line;

    // Save current read position
    auto curr_g = fs_file.tellg();

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
    fs_file.seekg(curr_g);

    return deleted;
}

bool delete_dir(std::fstream& fs_file, const std::string& dir_name)
{
    bool deleted{};
    std::string fs_line;

    // Save current read position
    auto curr_g = fs_file.tellg();

    // Move read position to the beginning
    fs_file.seekg(0, std::ios::beg);

    while (read_line(fs_file, fs_line) && !deleted)
    {
        // Record is found
        if (fs_line.front() == DIR_RECORD_IDENTIFIER && fs_line.substr(1) == dir_name)
        {
            // Delete the record identifier
            delete_line(fs_file, fs_line);

            // Delete any additional records that were within the dir
            while (read_line(fs_file, fs_line))
            {
                char record_type = fs_line.front();
                std::string record_name = fs_line.substr(1);
                size_t found_index = record_name.find(dir_name);

                // If a record name contains the dir
                if (found_index == 0 && record_type != DELETED_RECORD_IDENTIFIER)
                {
                    if (record_type == FILE_RECORD_IDENTIFIER)
                    {
                        // Delete the record identifier
                        delete_line(fs_file, fs_line);

                        // Delete any additional content lines for file records
                        while (read_line(fs_file, fs_line) && fs_line.front() == RECORD_CONTENT_IDENTIFIER)
                            delete_line(fs_file, fs_line);
                    }
                    else if (record_type == DIR_RECORD_IDENTIFIER)
                    {
                        delete_line(fs_file, fs_line);
                    }
                }
            }

            deleted = true;
        }
    }

    // Restore read position
    fs_file.seekg(curr_g);

    return deleted;
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
        char record_type = fs_line.front();
        bool is_dir = record_type == DIR_RECORD_IDENTIFIER;
        std::string line_content = fs_line.substr(1);

        // If the record is a file ('@')/dir ('=')
        if (record_type == FILE_RECORD_IDENTIFIER || is_dir)
        {
            if (!is_internal_path_valid(line_content, is_dir))
            {
                fprintf(stderr, "%s Invalid record path \"%s\"\n",
                    VSFS_ERROR_PREFIX, line_content.c_str());
                return nullptr;
            }

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
                size_t found_index = curr_dir->find_by_name(subdir_name);
                dir* subdir;

                // If subdir does not exist
                if (found_index == curr_dir->get_children().size())
                {
                    // If an intermediate dir is not to be created, throw error
                    if (!is_dir && !create_intermediate_dirs)
                    {
                        fprintf(stderr, "%s FS dir \"%s\" could not be found for file \"%s\"\n",
                            VSFS_ERROR_PREFIX, subdir_name.c_str(), line_content.c_str());
                        return nullptr;
                    }
                    else
                    {
                        // Create intermediate subdir
                        curr_dir->add_child((subdir = new dir(subdir_name, line_content)));
                    }
                }
                else
                {
                    // If subdir does exist, reuse
                    subdir = dynamic_cast<dir*>(curr_dir->get_children().at(found_index));

                    // Check if record is a duplicate
                    if (subdir->get_path() == line_content)
                    {
                        fprintf(stderr, "%s FS dir \"%s\" already exists in %s\n",
                            VSFS_ERROR_PREFIX, line_content.c_str(),
                            (curr_dir == root ? "FS" : ("dir \"" + curr_dir->get_name() + "\"").c_str()));
                        return nullptr;
                    }
                }

                // Traverse down a level
                curr_dir = subdir;
                curr_path = curr_path.substr(curr_delim + 1);
            }

            // Loop exits, the algorithm is in the right dir level

            // Insert the file in the current dir
            if (!is_dir)
            {
                // If file is a duplicate
                if (curr_dir->find_by_name(curr_path) != curr_dir->get_children().size())
                {
                    fprintf(stderr, "%s FS file \"%s\" already exists in dir \"%s\"\n",
                        VSFS_ERROR_PREFIX, curr_path.c_str(), curr_dir->get_name().c_str());
                    return nullptr;
                }
                else
                {
                    // If the record read is a file, establish parent-child relationship
                    file* f = new file(curr_path, line_content);
                    curr_file = f;
                    curr_dir->add_child(f);
                    fs_records.push_back(curr_file);
                }
            }
            else
            {
                // If instead the record is a dir, it is already added
                fs_records.push_back(curr_dir);
            }
        }
        else if (record_type == RECORD_CONTENT_IDENTIFIER)
        {
            // If no file is currently being assessed, i.e., content is placed in incorrect location
            if (!curr_file)
            {
                // Record content is detached from any file
                fprintf(stderr, "%s No file for content to belong to \"%s...\"\n", VSFS_ERROR_PREFIX,
                    line_content.size() < 10 ? line_content.c_str() : line_content.substr(0, 10).c_str());
                return nullptr;
            }

            // Append the content records to the last assessed file
            curr_file->append_content(line_content + "\n");
        }
        else if (record_type != DELETED_RECORD_IDENTIFIER)
        {
            // If the record type is not one of the known ones
            fprintf(stderr, "%s Unknown record type %c\n", VSFS_ERROR_PREFIX, record_type);
            return nullptr;
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

            // As the notes file requires dir records to be present before any children records
            // Dirs get higher privilege
            int file1_rank = file1_dir ? 1 : 0;
            int file2_rank = file2_dir ? 1 : 0;

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

bool is_internal_path_valid(const std::string& path, bool is_dir)
{
    /*
     *  Not beginning/containing '..'
     *  Not beginning/containing '.'
     *  Not beginning with a '/'
     *  Not ending with a '/' for files, must end with a '/' for dirs
     */
    return (path.find("..") == std::string::npos
        && path.find('.') == std::string::npos
        && path.front() != PATH_SEPARATOR
        && (is_dir
        ? path.at(path.size() - 1) == PATH_SEPARATOR
        : path.at(path.size() - 1) != PATH_SEPARATOR));
}

int calculate_subdir(dir* rootdir)
{
    // Start at "1" for it to be compliant with Midnight Commander
    int subdir_count = 1;
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
