set CC=gcc
set CPP=g++
set LD=ld
set FLAGS=-g -Wall -Wextra -pedantic -Wno-missing-field-initializers
set SAN=
REM # -fsanitize=address -fsanitize=undefined"
%CPP% %FLAGS% %SAN% -c tests/test_pixfont.cpp -o build/test_pixfont.o -I . -I ../third_party/stb/
%CC% %FLAGS% %SAN% -c apg_pixfont.c -o build/apg_pixfont.o -I .
%CPP% %FLAGS% -o test_pixfont.exe build/apg_pixfont.o build/test_pixfont.o -lm

%CC% -o bakefont.exe utils/bake_font_array.c -I ../third_party/stb/ -lm
%CC% -g -o create_atlas.exe utils/create_atlas.c apg_pixfont.c -I ../third_party/stb/ -I . -lm
