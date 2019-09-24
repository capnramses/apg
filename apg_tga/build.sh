gcc -fsanitize=address -ggdb -Wall -pedantic -Werror -g -o demo main_opengl.c ../common/src/GL/glew.c -I ../common/include/ -lglfw -lpthread -lGL
gcc main.c -g