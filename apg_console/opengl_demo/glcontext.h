// Copyright Anton Gerdelan <antonofnote@gmail.com>. 2019
#pragma once

// this header is for internal sharing of glfw window and GL headers between gfx.c and input.c

#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern GLFWwindow* g_window;
