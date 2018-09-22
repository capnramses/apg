#!/bin/sh

# any error code causes script to exit with error code
set -e

echo "building maths library tests..."
cd apg_maths
gcc -std=99 test.c -lm
g++ test.cpp
echo "building maths library tests...SUCCESS"
