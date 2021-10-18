#ifndef VSFS_BOOK_H
#define VSFS_BOOK_H

#include <regex>

#include "vsfs_helpers.h"

int vsfs_book(int argc, char** argv) {
    std::string path = argv[2];
    std::fstream file;
    open_file(path, file, std::ios::in);

    // Match roman numerals followed by a all capital heading
    std::regex pattern("(^M{0,4}(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3}))\\.([[:blank:]]|[A-Z]{2,})+");
    std::smatch match;
    std::stringstream stream;
    std::string line;
    int i = 0;
    while (read_line(file, line)) {
        stream << line;
        if (i == 1) {
            std::string text = stream.str();
            if (std::regex_search(text, match, pattern)) {
                std::string str(match[0].first, match[0].second);
                std::cout << str << "\n";
            }
            stream.str(std::string());
        }
        i = (i + 1) % 2;
    }

    return EXIT_SUCCESS;
}

#endif // VSFS_BOOK_H
