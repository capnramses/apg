#!/bin/sh
echo "building maths library tests..."
cd apg_maths
gcc -std=99 test.c -lm
g++ test.cpp
echo "building maths library tests...SUCCESS"
