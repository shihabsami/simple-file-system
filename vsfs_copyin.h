#ifndef VSFS_COPYIN_H
#define VSFS_COPYIN_H

#include "vsfs_commands.h"

#include <fstream>
#include <sstream>

int vsfs_copyin(int argc, char** argv) {
    int exit_code = EXIT_FAILURE + COPYIN;

    // command structure validation
    if (!argv[2]) {
        fprintf(stderr, "No FS provided\n");
        return exit_code;
    } else if (!argv[3]) {
        fprintf(stderr, "No EF provided\n");
        return exit_code;
    } else if (!argv[4]) {
        fprintf(stderr, "No IF provided\n");
        return exit_code;
    } else if (argc > 5) {
            fprintf(stderr, "Arguments for command \"list\", expected: 4, received: %d\n", argc - 1);
        return exit_code;
    }

    std::string fs_path = argv[2];
    std::string ef_path = argv[3];
    std::string if_path = argv[4];

    try {
        std::ifstream if_file;
        if_file.exceptions(if_file.exceptions() | std::ifstream::failbit | std::ifstream::badbit);
        if_file.open(if_path);

        std::ofstream ef_file;
        ef_file.open(ef_path, std::ios::out | std::ios::app);
        if (ef_file.fail())
            throw std::ios_base::failure(std::strerror(errno));
        ef_file.exceptions(ef_file.exceptions() | std::ios::failbit | std::ifstream::badbit);
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.what());
        return exit_code;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_COPYIN_H
