#! /usr/bin/bash

prefix="valgrind\
 --leak-check=full\
 --show-leak-kinds=all\
 --track-origins=yes\
 ../vsfs"

echo "$prefix"
cmd="$1"
case "$cmd" in
  list)     $prefix "$@" ;;
  copyin)   $prefix "$@" ;;
  copyout)  $prefix "$@" ;;
  mkdir)    $prefix "$@" ;;
  rm)       $prefix "$@" ;;
  rmdir)    $prefix "$@" ;;
  defrag)   $prefix "$@" ;;
  *) exit 1 ;;
esac
