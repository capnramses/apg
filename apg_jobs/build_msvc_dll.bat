REM WARNING: this "call ..." is for use as a batch file from explorer. Don't call this in a terminal or it double-sets things and goes bonkers.
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

cl /LD apg_jobs.c

pause
