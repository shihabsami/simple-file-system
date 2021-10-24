#ifndef VSFS_COPYOUT_H
#define VSFS_COPYOUT_H

#include "vsfs_helpers.h"
#include "vsfs_constants.h"

int vsfs_copyout(int argc, char** argv)
{
    // Verify number of arguments
    if (argc != 5)
    {
        fprintf(stderr, "%s Arguments for command \"copyout\", expected 3, received %d\n",
            VSFS_ERROR_PREFIX, argc - 2);
        return EXIT_FAILURE;
    }

    std::string fs_path = argv[2], if_path = argv[3], ef_path = argv[4];
    std::fstream fs_file, ef_file;
    bool is_compressed{};

    // Open FS file in read/write/append mode
    int err_code = open_fs(fs_path, fs_file, is_compressed, std::ios::in | std::ios::out);
    if (err_code != EXIT_SUCCESS)
        return err_code;

    if (!record_exists(if_path, fs_file))
    {
        fprintf(stderr, "%s IF could not be found \"%s\"\n", VSFS_ERROR_PREFIX, if_path.c_str());
        return ENOENT;
    }

    // Create temporary EF in case file is binary and is to be decoded later
    std::stringstream tmp_stream;
    err_code = run_command("mktemp", &tmp_stream);
    if (err_code != EXIT_SUCCESS)
    {
        fprintf(stderr, "%s Failed creating necessary /tmp file\n", VSFS_ERROR_PREFIX);
        return EXIT_FAILURE;
    }

    // Create intermediate directories if needed
    err_code = run_command(("touch " + ef_path).c_str(), nullptr);
    if (err_code != EXIT_SUCCESS)
    {
        fprintf(stderr, "%s Failed creating intermediate directories for EF \"%s\"\n",
            VSFS_ERROR_PREFIX, ef_path.c_str());
        return EXIT_FAILURE;
    }

    std::string tmp_ef = tmp_stream.str().substr(0, tmp_stream.str().find('\n'));
    bool written = false;

    // Write from IF out to temporary file
    std::string fs_line;
    while (read_line(fs_file, fs_line) && !written)
    {
        if (fs_line.front() == FILE_RECORD_IDENTIFIER && fs_line.substr(1) == if_path)
        {
            // Open tmp EF file in write mode
            err_code = open_ef(tmp_ef, ef_file, std::ios::out | std::ios::trunc, false);
            if (err_code != EXIT_SUCCESS)
                return err_code;

            // Write the IF's content to EF one line at a time
            while (read_line(fs_file, fs_line) && fs_line.front() == RECORD_CONTENT_IDENTIFIER)
                ef_file << fs_line.substr(1) << "\n";

            written = true;
        }
    }

    // If FS was found zipped, re-zip it
    if (is_compressed)
        gzip_fs(true, fs_path);


    ef_file.clear();
    ef_file.close();

    // If the tmp file was found to be ASCII, simply move it
    if (is_file_ascii(tmp_ef))
    {
        // Moving erases the EF's content if existing, as expected
        err_code = run_command(("mv \"" + tmp_ef + "\" \"" + ef_path + "\"").c_str(), nullptr);
        if (err_code != EXIT_SUCCESS) {
            fprintf(stderr, "%s EF could not be created \"%s\"\n", VSFS_ERROR_PREFIX, ef_path.c_str());
            return err_code;
        }
    }
    else
    {
        // Else decode the file
        return base64_decode(tmp_ef, ef_path);
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYOUT_H
