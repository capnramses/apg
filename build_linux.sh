#!/bin/sh

# any error code causes script to exit with error code
set -e
# echo everything
set -x

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
$CC $FLAGS tests/main.c apg_console.c apg_pixfont.c -I ./
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
echo "building apg_line_chart tests..."
cd apg_line_chart
$CC $FLAGS -std=c99 -I./ -I../third_party/stb/ apg_linechart.c -lm
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
$CC $FLAGS -std=c99 -I./ tests/main.c apg_mod.c -lm
cd ..

#
# [apg_pixfont]
#
echo "building apg_pixfont tests..."
cd apg_pixfont
$CPP -c tests/test_pixfont.cpp -o tests/test_pixfont.o -I ./ -I ../third_party/stb/
$CC $FLAGS -c apg_pixfont.c -o apg_pixfont.o -I ./
$CC $FLAGS $SAN -o test_pixfont.bin apg_pixfont.o tests/test_pixfont.o -lm
$CC $SAN -o bakefont.bin utils/bake_font_array.c -I ../third_party/stb/ -lm
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
