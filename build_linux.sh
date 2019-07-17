#!/bin/sh

# any error code causes script to exit with error code
set -e

echo "building data structures tests..."
cd apg_data_structs
gcc test.c

echo "building interpolation tests..."
cd apg_interp
gcc test.c -lm

echo "building maths library tests..."
cd apg_maths
gcc test.c -lm

echo "building wav library tests..."
cd apg_wav
gcc main_write.c apg_wav.c -lm
gcc main_read.c apg_wav.c -lm
