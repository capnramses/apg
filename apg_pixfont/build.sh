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
LD=ld
FLAGS="-g -Wall -Wextra -pedantic"
SAN="" # -fsanitize=address -fsanitize=undefined"
$CPP $FLAGS $SAN -c tests/test_pixfont.cpp -o tests/test_pixfont.o -I ./ -I ../third_party/stb/
$CC $FLAGS $SAN -c apg_pixfont.c -o apg_pixfont.o -I ./
$CC -o test_pixfont.bin apg_pixfont.o tests/test_pixfont.o -lm

$CC -o bakefont.bin utils/bake_font_array.c -I ../third_party/stb/ -lm

printf '%b%s\n%s\n%s\n%b\n' $colour_pass $horiz_div "Build SUCCESS" $horiz_div $colour_default
