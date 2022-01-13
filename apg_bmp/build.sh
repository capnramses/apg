#!/bin/bash
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_read_bmp.bin test_code/main_read.c apg_bmp.c -I./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_write_bmp.bin test_code/main_write.c apg_bmp.c -I./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
