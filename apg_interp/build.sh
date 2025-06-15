#!/bin/sh

# any error code causes script to exit with error code
set -e

CC=clang
CPP=clang++
LD=ld
FLAGS="-g -Wall -Wextra -pedantic -Werror"
SAN="-fsanitize=address -fsanitize=undefined"

$CC $FLAGS $SAN -std=c99 -I ./ tests/test.c -lm
