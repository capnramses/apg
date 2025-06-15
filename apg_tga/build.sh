#!/bin/bash

# any error code causes script to exit with error code
set -e

CC=clang
CPP=clang++
LD=ld
FLAGS="-g -Wall -Wextra -pedantic -Werror"
SAN="-fsanitize=address -fsanitize=undefined"

$CC $FLAGS -o test_read_tga test_code/main_read.c -I ./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT -lm
$CC $FLAGS -o test_write_tga test_code/main_write.c -I ./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT -lm