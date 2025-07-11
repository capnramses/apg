#!/bin/sh

# any error code causes script to exit with error code
set -e
# echo everything
#set -x

colour_pass="\033[32;1m"
colour_default="\033[0m"
horiz_div="========================"

CC=clang
CPP=clang++
FLAGS="-fsanitize=address -fsanitize=undefined -Wall -Wextra -Werror -pedantic -g"

#
# [apg]
#
echo "building apg tests..."
cd apg
$CC $FLAGS -o test_rle_compress_file.bin tests/rle_compress.c -I ./
$CC $FLAGS -o test_rle_string.bin tests/rle_test.c -I ./
$CC $FLAGS -o test_hash.bin tests/hash_test.c -I ./
$CC $FLAGS -o test_is_file.bin tests/is_file.c -I ./
$CC $FLAGS -o test_dir_list.bin tests/dir_list.c -I ./
cd ..

#
# [apg_bmp]
#
echo "building apg_bmp tests..."
cd apg_bmp
$CC $FLAGS -o test_read_bmp.bin test_code/main_read.c apg_bmp.c -I./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
$CC $FLAGS -o test_write_bmp.bin test_code/main_write.c apg_bmp.c -I./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
cd ..

#
# [apg_console]
#
echo "building apg_console tests..."
cd apg_console
cp ../apg_pixfont/apg_pixfont.c ./
cp ../apg_pixfont/apg_pixfont.h ./
cp ../apg_unicode/apg_unicode.c ./
cp ../apg_unicode/apg_unicode.h ./
$CC $FLAGS tests/main.c apg_console.c apg_pixfont.c apg_unicode.c -I ./
cd ..

# NOTE -- no tests for apg_gldb at the moment (opengl is a pain to set up on build servers)
# [apg_gldb]
#

#
# [apg_interp]
#
echo "building apg_interp tests..."
cd apg_interp
$CC $FLAGS -std=c99 -I ./ tests/test.c -lm
cd ..

#
# [apg_jobs]
#
echo "building apg_jobs tests..."
cd apg_jobs
clang $SANS $FLAGS tests/main.c -I ./ apg_jobs.c -pthread
cd ..

#
# [apg_linechart]
#
echo "building apg_plot tests..."
cd apg_plot
$CC $FLAGS -std=c99 -D_APG_PLOT_UNIT_TEST -I./ -I../third_party/stb/ apg_plot.c -lm
cd ..

#
# [apg_maths]
#
echo "building apg_maths tests..."
cd apg_maths
$CC $FLAGS -std=c99 -I./ tests/test.c apg_maths.c -lm
cd ..

#
# [apg_mod]
#
echo "building apg_mod tests..."
cd apg_mod
bash ./build.sh
cd ..

#
# [apg_pixfont]
#
echo "building apg_pixfont tests..."
cd apg_pixfont
bash ./build.sh
cd ..

#
# [apg_tga]
#
echo "building apg_tga tests..."
cd apg_tga
$CC $FLAGS -o test_read_tga test_code/main_read.c -I ./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
$CC $FLAGS -o test_write_tga test_code/main_write.c -I ./ -I test_code/ -I ../third_party/stb/ -DAPG_TGA_DEBUG_OUTPUT
cd ..

#
# [apg_unicode]
#
echo "building apg_unicode tests..."
cd apg_unicode
$CC $FLAGS -o test_unicode.bin tests/main.c apg_unicode.c -I ./
cd ..

#
# [apg_wav]
#
echo "building apg_wav tests..."
cd apg_wav
$CC $FLAGS -I./ -Itests/ tests/main_write.c apg_wav.c -lm
$CC $FLAGS -I./ -Itests/ tests/main_read.c apg_wav.c -lm
cd ..

printf '%b%s\n' $colour_pass $horiz_div
printf '%b%s\n' $colour_pass "Compile Check PASSED"
printf '%b%s\n%b\n' $colour_pass $horiz_div $colour_default
