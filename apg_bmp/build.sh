#!/bin/bash
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_read_bmp test_code/main.c -I./ -Itest_code/ apg_bmp.c -DAPG_BMP_DEBUG_OUTPUT
