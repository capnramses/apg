#!/bin/bash
gcc -o test_rle_compress_file.bin tests/rle_compress.c -I ./ -Wall
gcc -o test_rle_string.bin tests/rle_test.c -I ./ -Wall
