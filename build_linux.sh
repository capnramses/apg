#!/bin/sh

# any error code causes script to exit with error code
set -e
# echo everything
set -x

CC=clang
CPP=clang++
FLAGS="-fsanitize=address -fsanitize=undefined -Wall -Wextra -Werror -pedantic -g"

# TODO(Anton) need to recreate test cases for apg

echo "building apg_bmp tests..."
cd apg_bmp
$CC $FLAGS -o test_read_bmp test_code/main_read.c -I./ -Itest_code/ apg_bmp.c -DAPG_BMP_DEBUG_OUTPUT
$CC $FLAGS -o test_write_bmp test_code/main_write.c -I./ -Itest_code/ apg_bmp.c -DAPG_BMP_DEBUG_OUTPUT
cd ..


echo "building apg_console tests..."
cd apg_console
cp ../apg_pixfont/apg_pixfont.c ./
cp ../apg_pixfont/apg_pixfont.h ./
$CC $FLAGS tests/main.c apg_console.c apg_pixfont.c -I ./
cd ..

echo "building apg_interp tests..."
cd apg_interp
$CC $FLAGS -std=c99 -I./ tests/test.c -lm
cd ..

echo "building apg_maths tests..."
cd apg_maths
$CC $FLAGS -std=c99 -I./ tests/test.c -lm
cd ..

echo "building apg_pixfont tests..."
cd apg_pixfont
$CC $FLAGS -std=c99 -o apg_pixfont.o -I./ -c apg_pixfont.c
$CPP $FLAGS -I./ -Itests/ tests/test_pixfont.cpp apg_pixfont.o -lm
cd ..

echo "building apg_tga tests..."
cd apg_tga
$CC $FLAGS -o test_read_tga test_code/main_read.c -I./ -Itest_code/ -DAPG_TGA_DEBUG_OUTPUT
$CC $FLAGS -o test_write_tga test_code/main_write.c -I./ -Itest_code/ -DAPG_TGA_DEBUG_OUTPUT
cd ..

echo "building wav library tests..."
cd apg_wav
$CC $FLAGS -I./ -Itests/ tests/main_write.c apg_wav.c -lm
$CC $FLAGS -I./ -Itests/ tests/main_read.c apg_wav.c -lm
cd ..
