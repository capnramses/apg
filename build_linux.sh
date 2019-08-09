#!/bin/sh

# any error code causes script to exit with error code
set -e

echo "building data structures tests..."
cd apg_data_structs
gcc -std=c99 test.c
cd ..

echo "building interpolation tests..."
cd apg_interp
gcc -std=c99 test.c -lm
cd ..

echo "building maths library tests..."
cd apg_maths
gcc -std=c99 test.c -lm
cd ..

echo "building pixfont library tests..."
cd apg_pixfont
g++ test_pixfont.cpp apg_pixfont.c
cd ..

echo "building wav library tests..."
cd apg_wav
gcc -std=c99 main_write.c apg_wav.c -lm
gcc -std=c99 main_read.c apg_wav.c -lm
cd ..