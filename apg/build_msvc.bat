call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM "we recommend you compile by using either the /W3 or /W4 warning level"
REM C4221 is nonstandard extension used in struct literals.
REM C4996 is strdup -> _strdup deprecation warning
set COMPILER_FLAGS=/W4 /D_CRT_SECURE_NO_WARNINGS /wd4221 /wd4996 /MTd
set LINKER_FLAGS=/out:hash_test.exe
REM set LIBS=imagehlp.lib

set BUILD_DIR=".\build"
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

set I=/I ..\ ^
/I ..\tests\

set SRC=..\tests\hash_test.c

cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

pause
