#!/bin/bash
clang -o test_rle_compress_file.bin tests/rle_compress.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g
clang -o test_rle_string.bin tests/rle_test.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g
clang -o test_hash.bin tests/hash_test.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g
clang -o test_is_file.bin tests/is_file.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g 
clang -o test_dir_list.bin tests/dir_list.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g
clang -o test_rand.bin tests/rand_r_test.c -I ./ -Wall -Wextra -pedantic -fsanitize=address -g
