REM building apg_tga tests...
gcc -g -Wall -Wextra -pedantic -Wfatal-errors -o test_read_tga.exe test_code/main_read.c -I . -Itest_code/ -DAPG_TGA_DEBUG_OUTPUT
gcc -g -Wall -Wextra -pedantic -Wfatal-errors -o test_write_tga.exe test_code/main_write.c -I . -Itest_code/ -DAPG_TGA_DEBUG_OUTPUT
pause
