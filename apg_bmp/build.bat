REM building apg_bmp tests with clang.exe...
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_read_bmp.exe test_code/main_read.c -I./ -Itest_code/ apg_bmp.c -DAPG_BMP_DEBUG_OUTPUT
clang -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic -o test_write_bmp.exe test_code/main_write.c -I./ -Itest_code/ apg_bmp.c -DAPG_BMP_DEBUG_OUTPUT
pause