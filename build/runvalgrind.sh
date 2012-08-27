#!/bin/sh

valgrind --leak-check=full --show-reachable=yes --suppressions=valgrind.supp\
         ./a.out

