#!/bin/bash

# any error code causes script to exit with error code
set -e


CC=clang
CPP=clang++
LD=ld
FLAGS="-g -Wall -Wextra -pedantic -Werror"
SAN="-fsanitize=address -fsanitize=undefined"

$CC -o test_unicode.bin tests/main.c apg_unicode.c -I ./
