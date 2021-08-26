REM WARNING: this "call ..." is for use as a batch file from explorer. Don't call this in a terminal or it double-sets things and goes bonkers.
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM "we recommend you compile by using either the /W3 or /W4 warning level"
REM C4221 is nonstandard extension used in struct literals.
set COMPILER_FLAGS=/W4 /D_CRT_SECURE_NO_WARNINGS /wd4221
set LINKER_FLAGS=/out:test_apg_jobs.exe
set LIBS=

set BUILD_DIR=".\build"
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

set I=/I ..\src\ /I ..\src\apg\ /I..\src\stb\ /I..\src\glad\include\

set SRC=..\main.c ^
..\apg_jobs.c

cl %COMPILER_FLAGS% %SRC% %I% /link %LINKER_FLAGS% %LIBS%

copy test_apg_jobs.exe ..\

pause
