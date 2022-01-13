#!/bin/bash
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_read_tga.bin test_code/main_read.c -I ./ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_write_tga.bin test_code/main_write.c -I ./ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
