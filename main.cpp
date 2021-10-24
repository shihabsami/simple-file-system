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
    // Run the appropriate command
    try
    {
        if (!argv[1])
        {
            fprintf(stderr, "%s No commands provided\n", VSFS_ERROR_PREFIX);
            return EXIT_FAILURE;
        }
        else if (strcmp(argv[1], commands[LIST]) == 0)
        {
            return vsfs_list(argc, argv);
        }
        else if (strcmp(argv[1], commands[COPYIN]) == 0)
        {
            return vsfs_copyin(argc, argv);
        }
        else if (strcmp(argv[1], commands[COPYOUT]) == 0)
        {
            return vsfs_copyout(argc, argv);
        }
        else if (strcmp(argv[1], commands[MKDIR]) == 0)
        {
            return vsfs_mkdir(argc, argv);
        }
        else if (strcmp(argv[1], commands[RM]) == 0)
        {
            return vsfs_rm(argc, argv);
        }
        else if (strcmp(argv[1], commands[RMDIR]) == 0)
        {
            return vsfs_rmdir(argc, argv);
        }
        else if (strcmp(argv[1], commands[DEFRAG]) == 0)
        {
            return vsfs_defrag(argc, argv);
        }
        else
        {
            fprintf(stderr, "%s Unknown command \"%s\"\n", VSFS_ERROR_PREFIX, argv[1]);
            return EXIT_FAILURE;
        }
    }
    catch (const std::exception& exception)
    {
        // Very less likely to occur
        // Yet is caught and output printed for better understanding of any outstanding error
        fprintf(stderr, "%s %s\n", VSFS_ERROR_PREFIX, exception.what());
        return EXIT_FAILURE;
    }
}
