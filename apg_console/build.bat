set CC=gcc
set CPP=g++
set LD=ld
set FLAGS=-g -Wall -Wextra -pedantic -Werror
REM SAN=-fsanitize=address -fsanitize=undefined

cp ../apg_pixfont/apg_pixfont.c ./
cp ../apg_pixfont/apg_pixfont.h ./

%CC% %FLAGS% %SAN% tests/main.c apg_console.c apg_pixfont.c -I .
