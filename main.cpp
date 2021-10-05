#include <iostream>

#include "vsfs_list.h"
#include "vsfs_copyin.h"
#include "vsfs_test.h"

int main(int argc, char** argv) {
    /*
     * TODO
     *  - VSFS list FS
     *  - VSFS copyin FS EF IF
     *  - VSFS copyout FS IF EF
     *  - VSFS mkdir FS ID
     *  - VSFS rm FS IF
     *  - VSFS rmdir FS ID
     *  - VSFS defrag FS
     *  - VSFS index FS
     */

    try {
        if (!argv[1]) {
            fprintf(stderr, "No commands found\n");
            return EXIT_FAILURE;
        } else if (strcmp(argv[1], "list") == 0) {
            return vsfs_list(argc, argv);
        } else if (strcmp(argv[1], "copyin") == 0) {
            return vsfs_copyin(argc, argv);
        } else if (strcmp(argv[1], "test") == 0) {
            return vsfs_test(argc, argv);
        }
    } catch (...) {
        fprintf(stderr, "Something went wrong\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
