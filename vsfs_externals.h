#ifndef VSFS_EXTERNALS_H
#define VSFS_EXTERNALS_H

#include <sstream>

// Run a system command and get output if required
int run_command(const char* command, std::stringstream* output)
{
    std::string command_formatted = command;

    // If output is to be discarded
    if (!output)
        command_formatted.append(" > /dev/null 2>&1");

    // Open the process
    FILE* pipe = popen(command_formatted.c_str(), "r");
    if (!pipe)
        return EXIT_FAILURE;

    // If output is to be captured
    if (output)
    {
        // Buffer to store the output chunks
        std::array<char, 128> buffer{};
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            *output << buffer.data();
    }

    return pclose(pipe);
}

// Encode a file using the base64 command at the given path and store encoded data in the stream reference
int base64_encode(const std::string& path, std::stringstream& encoded_data)
{
    // Wrap content at 253 (' ' + content + '\n' or 1 + 253 + 1)
    std::string command = "base64 \"" + path + "\" -w " + std::to_string(MAXIMUM_RECORD_LENGTH - 2);

    int return_val = run_command(command.c_str(), &encoded_data);
    if (return_val != EXIT_SUCCESS)
        fprintf(stderr, "%s Failed encoding file \"%s\"\n", VSFS_ERROR_PREFIX, path.c_str());

    return return_val;
}

// Decode a file using the base64 command that is present at "from" and move to "to"
int base64_decode(const std::string& from, const std::string& to)
{
    std::string command = "base64 -d \"" + from + "\" > \"" + to + "\" && rm \"" + from + "\"";

    int return_val = run_command(command.c_str(), nullptr);
    if (return_val != EXIT_SUCCESS)
        fprintf(stderr, "%s Failed decoding file \"%s\"\n", VSFS_ERROR_PREFIX, to.c_str());

    return return_val;
}

// Zip or unzip FS at the given path using the gzip command
int gzip_fs(bool do_zip, std::string& fs_path)
{
    std::string command = (do_zip ? "gzip " : "gzip -d ") + fs_path;

    int return_val = run_command(command.c_str(), nullptr);
    if (return_val != EXIT_SUCCESS)
    {
        fprintf(stderr, "%s Failed %sping FS \"%s\"\n", VSFS_ERROR_PREFIX,
            do_zip ? "zip" : "unzip", fs_path.c_str());
    }
    else
    {
        // FS path should no longer contain ".gz" if unzipped and would be appended otherwise
        fs_path = do_zip ? fs_path + "." + GZ_EXTENSION : fs_path.substr(0, fs_path.find(GZ_EXTENSION) - 1);
    }

    return return_val;
}

// Check whether the file is in ASCII format
bool is_file_ascii(const std::string& path)
{
    std::stringstream output;
    run_command(("file " + path).c_str(), &output);

    // Transform to upper-case for case-insensitive comparison
    std::string output_str = output.str();
    std::transform(output_str.begin(), output_str.end(), output_str.begin(),
        [](unsigned char c)
        { return std::toupper(c); });

    // If the output specified the file as ASCII
    return output_str.find("ASCII") != std::string::npos;
}

#endif // VSFS_EXTERNALS_H
