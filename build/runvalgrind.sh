#!/bin/sh

current=$(pwd)

cd "./$(dirname $1)"
valgrind --leak-check=full --show-reachable=yes\
         --suppressions=$current/valgrind.supp\
         "./$(basename $1)"

