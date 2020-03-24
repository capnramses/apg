CC=clang
CPP=clang++
LD=ld
FLAGS="-g -Wall -Wextra -pedantic -Werror"
SAN="" # -fsanitize=address -fsanitize=undefined"
$CPP $FLAGS $SAN -c tests/test_pixfont.cpp -o tests/test_pixfont.o -I ./
$CC $FLAGS $SAN -c apg_pixfont.c -o apg_pixfont.o -I ./
$CPP apg_pixfont.o tests/test_pixfont.o -lm
