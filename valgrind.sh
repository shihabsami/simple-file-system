make clean
make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./vsfs copyout FS.notes dir1/dir2 myfile

