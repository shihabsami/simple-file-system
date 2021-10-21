#include <iostream>

#include "vsfs_list.h"
#include "vsfs_copyin.h"
#include "vsfs_copyout.h"
#include "vsfs_mkdir.h"
#include "vsfs_rm.h"
#include "vsfs_rmdir.h"
#include "vsfs_defrag.h"

int main(int argc, char** argv)
{
    try
    {
        if (!argv[1])
        {
            fprintf(stderr, "%s No commands found\n", VSFS_ERROR_PREFIX);
            return EXIT_FAILURE;
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            return vsfs_list(argc, argv);
        }
        else if (strcmp(argv[1], "copyin") == 0)
        {
            return vsfs_copyin(argc, argv);
        }
        else if (strcmp(argv[1], "copyout") == 0)
        {
            return vsfs_copyout(argc, argv);
        }
        else if (strcmp(argv[1], "mkdir") == 0)
        {
            return vsfs_mkdir(argc, argv);
        }
        else if (strcmp(argv[1], "rm") == 0)
        {
            return vsfs_rm(argc, argv);
        }
        else if (strcmp(argv[1], "rmdir") == 0)
        {
            return vsfs_rmdir(argc, argv);
        }
        else if (strcmp(argv[1], "defrag") == 0)
        {
            return vsfs_defrag(argc, argv);
        }
        else
        {
            fprintf(stderr, "%s Invalid command: %s\n", VSFS_ERROR_PREFIX, argv[1]);
            return EXIT_FAILURE;
        }
    }
    catch (...)
    {
        fprintf(stderr, "%s Unknown error\n", VSFS_ERROR_PREFIX);
        return EXIT_FAILURE;
    }
}
