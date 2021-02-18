@echo off

REM NOTE: Uncomment whichever one that you have installed!
REM call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 13.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

REM The /Z7 option produces object files that also contain full symbolic debugging information for use with the debugger
REM /Fc Causes the compiler to display the full path of source code files passed to the compiler in diagnostics.
REM /Fm outputs a map file
REM /Fd outputs a PDB file /MTd link with LIBCMTD.LIB debug lib /F<num> set stack size /W<n> set warning level (default n=1) /Wall enable all warnings /wd<n> disable warning n
REM language standard options are: /std:c++14 /std:c++17 /std:c++latest /std:c11 /std:c17
set COMPILER_FLAGS=/MTd /std:c11 /nologo /Wall /FC /Fm /Z7 /wd4061 /wd4100 /wd4201 /wd4204 /wd4255 /wd4365 /wd4464 /wd4505 /wd4514 /wd4530 /wd4577 /wd4668 /wd4710 /wd4820 /wd4996 /wd5045 /wd5100
set LINKER_FLAGS=/INCREMENTAL:NO /opt:ref
REM set LINKER_PATHS=/LIBPATH:"../"
REM set SYSTEM_LIBS=
REM set LOCAL_LIBS=

set BUILD_DIR=".\tmp"
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

cl %COMPILER_FLAGS% ^
../test.c ../../apg_maths.c ^
/I ../ /I ../../ ^
/link %LINKER_PATHS% %LINKER_FLAGS% %LOCAL_LIBS% %SYSTEM_LIBS%

popd
copy %BUILD_DIR%\test.exe .\test_msvc.exe
echo Done
