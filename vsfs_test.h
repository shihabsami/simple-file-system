#ifndef VSFS_TEST_H
#define VSFS_TEST_H

/*
 * Experiment ground
 */

void test_if_open(const char* path) {
    try {
        std::ifstream file;
        file.exceptions(file.exceptions() | std::ifstream::failbit | std::ifstream::badbit);
        file.open(path);

        if (file.peek() == EOF) {
            fprintf(stderr, "file is empty\n");
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.code().message().c_str());
    }
}

void write_test(const char* path, const char* content) {
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << content;
    file.close();
}

void replace_test(const char* path) {
    std::fstream file;
    file.open(path, std::fstream::in | std::fstream::out);

    try {
        std::string line;
        while (std::getline(file, line)) {
            printf("%s", line.c_str());
            if (line.at(0) == '@') {
                file.seekp(std::ios::off_type(file.tellp()) - (int) line.size() - 1, std::ios_base::beg);
                file.put('#');
            }
        }
    } catch (const std::ifstream::failure& failure) {
        fprintf(stderr, "I/O error: %s\n", failure.what());
        fprintf(stderr, "I/O error: %s\n", failure.code().message().c_str());
    }
}

void size_test(std::stringstream& content) {
    std::string line;
    int i = 0;
    std::stringstream format;
    while (std::getline(content, line)) {
        auto size = line.size();
        printf("size[%d]: %lu\n", i++, size);

        if (size > MAXIMUM_RECORD_LENGTH) {
            line.resize(MAXIMUM_RECORD_LENGTH);
            line.at(line.size() - 1) = '\n';
        } else if (size == MAXIMUM_RECORD_LENGTH) {
            line.at(size - 1) = '\n';
        } else if (size > 0 && line.at(size - 1) != '\n') {
            line.append("\n");
        }

        format << line;
    }

    printf("%s", format.str().c_str());
}

void copy_test(const char* path, const char* content) {
    std::fstream file;
    open_file(path, file, std::ios::in | std::ios::out | std::ios::app);

    std::stringstream temp;
    temp << file.rdbuf();

    file.close();
    open_file(path, file, std::ios::out | std::ios::trunc);
    file << content;
    file.close();

    printf("%s", temp.str().c_str());
}

void getline_test(const char* path) {
    std::fstream file;
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);
    open_file(path, file, std::ios::in | std::ios::out);
    bool fail = file.fail();
    bool eof = file.eof();
    bool bad = file.bad();
    try {
        std::string line;

        while (!file.eof() && file.peek() != EOF && std::getline(file, line)) {
            printf("%s", line.c_str());
            fail = file.fail();
            eof = file.eof();
            bad = file.bad();
        }
    } catch (std::ios::failure& failure) {
        fprintf(stderr, " Error, failbit: %d, eofbit: %d, badbit: %d\n", fail, eof, bad);
    }
}

int vsfs_test(int argc, char** argv) {
//    std::string path(argv[2]);
//    std::stringstream content;
//    std::ifstream file;
//    file.exceptions(file.exceptions() | std::ifstream::failbit | std::ifstream::badbit);
//
//    file.open(path);
//    content.str(std::string());
//    content << file.rdbuf();
//    file.close();
//
//    size_test(content);

//    copy_test(argv[2], argv[3]);

//    replace_test(argv[2]);

    getline_test(argv[2]);

    return EXIT_SUCCESS;
}

#endif // VSFS_TEST_H
