call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM "we recommend you compile by using either the /W3 or /W4 warning level"
REM C4221 is nonstandard extension used in struct literals.
REM C4996 is strdup -> _strdup deprecation warning
set COMPILER_FLAGS=/W4 /D_CRT_SECURE_NO_WARNINGS /wd4221 /wd4996 /MTd
REM set LIBS=imagehlp.lib

set BUILD_DIR=".\build"
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

set I=/I ..\ ^
/I ..\tests\

REM ==============================================================
REM HASH TEST
REM ==============================================================
set LINKER_FLAGS=/out:hash_test.exe
set SRC=..\tests\hash_test.c
cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

REM ==============================================================
REM GREEDY TEST
REM ==============================================================
set LINKER_FLAGS=/out:greedy_test.exe
set SRC=..\tests\greedy_test.c
cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

REM ==============================================================
REM MAKE BIG FILE
REM ==============================================================
set LINKER_FLAGS=/out:make_big_file.exe
set SRC=..\tests\make_big_file.c
cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

REM ==============================================================
REM BIG FILE TEST
REM ==============================================================
set LINKER_FLAGS=/out:bigfile_test.exe
set SRC=..\tests\test_big_file.c
cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

pause
