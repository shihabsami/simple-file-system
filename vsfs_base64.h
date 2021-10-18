#ifndef VSFS_BASE64_H
#define VSFS_BASE64_H

int decode(const std::string& fs_path) {
    std::string command = GZIP_DECODE_PREFIX + fs_path;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        fprintf(stderr, "%s Failed decoding gz FS: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());

    return pclose(pipe);
}

int encode(const std::string& fs_path) {
    std::string command = GZIP_ENCODE_PREFIX + fs_path;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        fprintf(stderr, "%s Failed encoding gz FS: %s\n", VSFS_ERROR_PREFIX, fs_path.c_str());

    return pclose(pipe);
}

int vsfs_base64(int argc, char** argv) {
    std::string path(argv[2]);
    if (path.find(".gz") == std::string::npos)
        encode(argv[2]);
    else
        decode(argv[2]);

    return EXIT_SUCCESS;
}

#endif // VSFS_BASE64_H
