gcc -o consoletests.exe -I src\ tests\main.c src\apg_console.c src\apg_pixfont.c

gcc -o demoopengl.exe -g -Wfatal-errors -DGLEW_STATIC ^
opengl_demo\main.c opengl_demo\gfx.c opengl_demo\utils.c src\apg_console.c src\apg_pixfont.c ^
-I ..\common\include\ -L ..\common\win64_gcc\ -I src\ ^
..\common\src\GL\glew.c ..\common\win64_gcc\libglfw3.a ^
-lm -lOpenGL32 -lgdi32
