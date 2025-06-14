/* apg_pixfont - C Pixel Font Utility v0.5.0
C99 Implementation
See header file for licence and instructions.
Anton Gerdelan 2019
============================================================== */
#include "apg_pixfont.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// clang-format off
#define A 0, 0
#define B A, A
#define C B, B
#define D C, C
#define E D, D
#define F E, E
#define G F, F
#define H G, G
#define Z 0, 255
#define Y 255, 255
#define X Z, Z
// Standard font.
static const int _font_img_w = 1024;
static const int _font_img_h = 16;
static const unsigned char _font_img[] = { H,H,H,H,H,H,F,D,A,Z,B,A,Z,B,255,B,X,C,Z,D,255,B,A,Z,B,255,C,A,255,B,A,Z,B,255,D,X,A,255,B,A,Z,B,255,B,X,D,B,255,B,A,Z,B,255,C,B,255,H,H,F,E,C,B,255,G,F,E,A,Z,B,Z,B,255,Z,A,X,A,X,A,X,D,255,B,Z,B,255,Z,A,X,B,255,B,Z,B,255,Z,A,X,C,X,B,255,B,Z,B,255,Z,A,X,A,X,D,255,B,Z,B,255,Z,A,X,B,255,F,E,D,255,Z,G,D,Z,A,255,C,Z,D,B,A,Z,A,Z,E,B,A,255,H,C,A,255,C,B,255,Z,C,255,G,E,B,A,Z,F,C,B,A,Z,G,E,255,B,A,Z,B,255,B,X,C,Z,D,255,B,A,Z,B,255,C,A,255,B,A,Z,B,255,C,A,Z,B,X,A,255,B,A,Z,B,255,B,X,D,B,255,B,A,Z,B,255,C,B,255,E,D,C,B,255,B,Z,A,255,C,Z,Y,D,B,Z,B,Z,E,B,Z,A,Z,Y,B,255,B,Y,255,A,Z,Y,B,Y,A,Y,Y,255,A,Y,255,A,Y,Y,255,A,Y,255,A,Z,Y,E,Z,Y,A,Z,Y,A,Z,Y,A,Y,Y,A,Z,Y,A,Y,Y,A,Y,Y,255,Z,Y,Y,A,Y,Y,Z,A,X,Y,Y,Z,Y,Y,Z,A,X,B,Z,A,X,A,Z,A,Y,255,A,Y,Y,A,Z,Y,A,Y,Y,A,Z,Y,A,Y,Y,255,Z,A,X,A,X,A,X,A,X,A,X,Y,Y,Z,Y,A,Z,B,Z,Y,A,Z,A,Z,C,255,C,A,255,C,B,A,Z,C,Z,255,C,255,B,A,Z,B,A,255,A,Y,B,255,E,D,255,E,C,A,255,A,Z,B,Z,C,A,Z,D,C,Z,B,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,255,A,Y,255,A,Y,Y,255,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,A,Y,255,A,255,A,Z,A,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,C,Z,Y,A,255,A,X,A,X,A,X,A,X,A,X,B,A,Y,B,Z,B,Z,B,255,Z,A,X,A,X,A,X,D,255,B,Z,B,255,Z,A,X,B,255,B,Z,B,255,Z,A,X,A,X,A,X,B,255,B,Z,B,255,Z,A,X,A,X,D,255,B,Z,B,255,Z,A,X,B,255,C,A,255,Z,A,Z,Y,255,C,Y,255,D,C,B,A,255,C,B,255,Z,A,255,X,B,Z,A,255,B,255,B,A,255,B,Z,B,A,255,B,Z,D,B,A,Z,A,Z,A,Z,A,Y,A,Z,A,X,A,Z,A,255,Z,A,255,B,Z,A,X,A,X,A,X,A,Z,C,B,A,Z,C,Z,B,Z,A,X,A,X,A,X,A,X,A,X,A,X,B,Z,B,Z,A,X,A,Z,A,Z,B,A,255,A,255,A,255,A,255,B,Z,255,Z,255,Z,255,A,255,Z,A,X,A,X,A,X,A,X,A,Z,A,Z,A,Z,A,X,A,X,A,Z,A,255,Z,A,255,A,Z,B,X,B,A,255,B,A,255,D,C,A,Z,C,B,A,Z,C,255,A,255,B,A,Z,D,A,255,B,255,E,D,255,E,C,Z,B,255,B,A,255,B,Z,255,Z,C,Y,255,D,B,255,A,X,A,X,A,X,A,X,A,X,A,X,Z,A,Z,A,X,B,Z,B,Z,B,Z,B,A,Z,B,Z,B,Z,B,Z,B,255,A,255,Z,255,A,255,Z,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,Y,A,Z,A,255,E,A,255,F,A,Z,E,B,A,Z,E,B,A,Z,C,A,Z,A,255,C,255,A,Z,D,C,B,Z,C,A,Z,Y,Y,X,A,Z,A,255,A,255,Z,A,Z,B,Z,B,A,Z,A,Z,Y,Y,A,Z,D,B,A,Z,A,Z,255,A,255,A,Z,B,A,Z,B,X,A,255,A,255,B,Z,C,X,A,X,A,X,B,A,255,B,Z,B,Y,Y,A,Z,C,255,Z,A,Y,Z,A,X,A,X,B,Z,A,X,B,Z,B,Z,B,Z,A,Z,A,Z,B,A,255,A,255,A,255,A,255,B,X,X,255,A,255,Z,A,X,A,X,A,X,A,X,B,A,Z,A,Z,A,X,A,X,A,Z,A,255,Z,A,X,B,Z,A,255,B,A,255,B,A,255,D,B,A,Y,Y,Z,Y,255,A,Z,Y,255,A,Y,Y,A,Y,255,A,Z,B,Z,Y,255,Z,Y,255,A,Z,255,B,Z,255,A,Z,A,255,Z,B,Z,255,Z,A,Y,Y,A,Z,Y,A,Y,Y,A,Z,Y,255,X,255,A,Z,Y,A,Y,Y,A,255,A,X,A,X,A,X,A,X,A,X,Y,Y,A,255,B,255,B,A,255,B,255,A,255,A,255,B,X,Z,A,255,A,255,Z,A,255,B,255,A,Z,A,X,A,X,A,X,A,X,A,X,A,X,Z,A,Z,B,Z,B,Z,B,Z,B,Z,B,A,Z,B,Z,B,Z,B,Z,B,255,A,255,Z,255,A,255,Z,A,X,A,X,A,X,A,X,A,Z,A,255,Z,A,255,A,Y,Z,A,X,A,X,A,X,A,Z,A,255,Z,A,255,A,255,A,255,A,255,A,Z,Y,255,A,Y,Y,A,Y,Y,A,Y,Y,A,Y,Y,A,Y,Y,A,Y,255,A,Z,Y,255,A,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,255,B,Y,B,Y,B,Y,B,A,X,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,C,Z,255,X,A,X,A,X,A,X,A,X,A,X,Z,255,A,255,A,X,A,255,A,X,A,255,A,Z,A,Y,255,D,C,255,C,B,255,Z,A,Z,Y,B,255,B,255,C,A,255,B,A,Z,B,Z,A,Z,Y,Y,B,A,Z,Y,Y,C,255,B,255,X,A,Z,B,A,255,B,Z,A,Y,Y,255,A,Y,255,A,Y,Y,B,Z,A,Z,Y,A,Z,Y,255,C,B,Z,C,B,Z,B,A,255,A,255,X,Z,Y,Y,Z,Y,255,A,255,B,Z,A,X,Y,A,Z,Y,A,Z,A,Y,Z,Y,Y,A,Z,B,A,255,A,Y,255,A,Z,B,X,X,X,Z,A,X,Y,255,A,255,A,X,Y,255,A,Z,Y,B,255,A,Z,A,X,A,X,A,Z,A,Z,B,Z,B,Z,A,Z,B,A,Z,B,Z,D,B,Z,A,X,A,X,B,Z,A,X,A,Z,A,255,B,255,A,X,A,Z,A,Z,B,A,255,A,Z,Y,A,255,B,X,Z,A,255,A,255,Z,A,X,A,X,A,255,A,Y,A,255,Z,B,A,255,B,255,A,X,A,X,A,Z,A,255,Z,A,255,A,Z,B,X,B,Z,B,A,Z,C,Z,B,Z,255,A,255,Z,A,255,A,Z,A,255,A,255,B,Y,Y,255,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,255,A,255,B,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,Z,B,Z,B,Z,B,Z,A,Z,Y,X,X,Z,A,X,A,X,A,X,A,X,A,Z,A,Z,A,X,X,A,X,A,X,A,X,A,Z,A,Z,A,Z,A,255,A,255,Z,A,Z,A,X,A,X,A,X,A,X,A,X,A,X,X,Z,B,Z,A,X,A,X,A,X,A,Z,A,Z,B,Z,B,Z,B,Z,B,Y,Y,A,255,A,255,Z,A,X,A,X,A,X,A,X,A,X,Y,Y,Z,A,255,A,255,A,X,A,X,A,X,A,X,A,X,255,A,255,Z,A,X,A,Y,X,X,A,X,A,Z,D,B,A,Z,C,A,Z,Y,Y,A,X,A,255,A,255,X,Z,C,255,B,Z,B,X,B,255,D,B,A,255,B,255,A,Y,A,Z,B,Z,B,A,Z,B,255,B,A,255,Z,A,Z,B,255,A,255,A,Z,B,Z,C,B,A,255,B,Y,Y,A,Z,B,A,255,A,Z,A,255,A,255,A,X,A,X,B,Z,A,X,B,Z,B,Z,A,X,A,Z,A,Z,B,A,255,A,255,A,255,A,255,B,X,X,A,Y,Z,A,X,B,Z,A,X,A,Z,B,Z,A,Z,A,Z,A,X,A,X,X,A,Y,255,B,255,B,255,B,255,B,A,Z,B,Z,D,B,Z,A,X,A,X,B,Z,A,X,Y,Y,Z,Y,255,A,255,A,X,A,Z,A,Z,B,A,255,A,Z,255,A,Z,B,X,Z,A,255,A,255,Z,A,X,A,X,A,255,A,255,B,A,Y,255,A,Z,B,255,A,Z,A,255,Z,A,255,X,A,Z,A,Z,A,Z,A,Y,255,A,Z,B,255,B,A,255,C,A,255,B,X,Z,A,255,A,255,Z,A,255,A,255,B,Z,A,X,A,X,A,X,A,X,A,X,A,X,Z,A,Z,B,Z,B,Z,B,Z,B,Z,B,A,Z,B,Z,B,Z,B,Z,B,255,A,255,Z,A,Y,Z,A,X,A,X,A,X,A,X,A,Z,A,255,Z,A,Y,A,255,Z,A,X,A,X,A,X,A,Z,A,Z,A,Z,A,255,A,255,A,255,A,255,A,X,A,X,A,X,A,X,A,X,A,X,Y,Y,Z,B,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,A,Z,B,Z,B,Z,B,Z,A,Z,A,Z,A,255,A,255,Z,A,X,A,X,A,X,A,X,A,Z,B,A,X,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,255,A,255,Z,Y,Z,A,X,A,Z,E,B,255,Z,A,255,X,Z,B,Z,A,255,C,Z,B,Z,B,X,B,255,D,B,Z,B,Z,A,Z,A,Z,B,255,A,255,Z,A,Z,B,255,A,255,A,X,A,Z,A,Z,A,Z,A,Z,B,255,D,255,C,Z,C,A,Z,B,Z,A,X,A,X,A,X,A,X,B,Z,B,Z,A,X,A,Z,A,Z,A,Z,A,255,A,255,A,255,A,255,B,Z,A,X,A,Y,Z,A,X,B,Z,A,Y,Z,A,X,A,Z,A,Z,A,Z,A,Z,A,255,Z,A,255,X,A,255,Z,B,255,A,Z,B,Z,C,255,B,255,D,B,Z,A,Y,Z,A,X,B,Z,A,X,B,A,255,B,255,A,X,A,Z,A,Z,B,A,255,A,X,A,255,B,Z,A,Z,A,255,A,255,Z,A,X,A,X,A,255,A,255,C,Z,A,255,A,255,Z,A,Y,A,255,Z,A,255,X,A,255,Z,A,255,A,X,B,A,255,B,255,B,A,255,C,A,255,B,A,Y,255,C,B,A,255,A,X,A,X,A,X,A,X,A,X,A,X,A,X,Z,A,Z,A,X,B,Z,B,Z,B,Z,B,A,Z,B,Z,B,Z,B,Z,B,255,A,255,Z,A,Y,Z,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,X,A,Z,A,Z,A,Z,Y,A,Z,A,X,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,X,A,Z,B,Z,B,Z,B,Z,B,Z,B,A,Z,B,Z,B,Z,B,Z,A,Z,A,Z,A,255,A,255,Z,A,X,A,X,A,X,A,X,A,Z,A,Z,B,255,A,255,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,X,255,A,255,Z,A,X,A,255,A,255,Z,A,Z,A,X,A,Z,D,B,A,Z,D,A,Y,255,C,Z,255,Z,C,Z,A,Z,D,A,255,C,A,255,B,Z,B,A,Y,255,A,Y,Y,255,Z,Y,Y,A,Y,255,B,Y,255,A,Y,255,A,Z,Y,B,255,B,Y,255,A,Z,255,A,Z,B,A,255,D,C,255,B,Y,255,A,255,A,X,Y,255,A,Z,Y,A,Y,Y,A,Y,Y,255,Z,B,A,Y,255,A,255,A,X,Y,Y,A,Y,A,Z,A,X,Y,Y,Z,A,X,A,Z,A,Y,255,A,255,B,A,Y,255,A,255,A,Z,A,Y,255,B,255,B,Y,255,B,255,B,255,Z,A,255,A,Z,A,Z,A,Z,Y,Y,Z,Y,B,A,255,A,Y,255,C,Z,Y,Y,C,Y,X,Y,255,A,Z,Y,255,A,Y,Y,A,Y,Y,A,255,B,Z,Y,255,Z,A,X,Y,Y,B,255,A,Z,A,255,Z,B,Z,A,Z,A,255,A,255,A,Y,255,A,Y,Y,A,Z,Y,A,255,B,A,Y,255,B,Y,A,Z,255,Z,A,Z,B,255,Z,A,255,A,Z,A,Y,Y,Z,Y,Y,A,Z,C,Z,C,A,Z,D,C,Y,255,A,255,A,X,A,X,A,X,A,X,A,X,A,X,Z,Y,A,Y,255,A,Y,Y,255,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,A,Y,255,A,255,A,Z,A,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,C,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,255,A,Z,B,X,255,A,Z,255,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,X,Z,Y,A,Y,Y,A,Y,Y,A,Y,Y,A,Y,Y,A,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,Z,Y,Y,A,Y,Y,A,255,A,255,A,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,C,255,Z,255,A,Z,255,Z,A,Y,Z,A,Y,Z,A,Y,Z,A,Y,Y,X,255,A,Z,Y,255,A,Y,Y,A,255,Z,255,A,Y,255,A,Z,Y,E,C,B,255,E,C,B,A,255,F,D,C,Z,G,B,A,255,G,A,Z,D,255,E,255,C,255,E,D,255,F,E,C,Z,G,E,D,C,B,A,Z,G,B,X,C,Z,C,B,A,Z,B,Z,F,D,C,A,Z,F,D,C,Z,H,B,A,Z,A,Z,C,B,Z,A,255,E,255,C,255,E,C,B,A,Z,F,E,C,A,Z,G,E,D,C,B,A,Z,G,A,Z,C,A,Z,C,B,A,Z,B,Z,H,G,E,D,C,B,A,Z,Y,C,B,A,Z,255,E,Z,C,Y,E,C,B,Y,F,E,C,A,Z,G,E,D,C,B,A,Z,G,A,Y,C,A,Y,D,Z,B,Z,H,H,H,H,H,H,H,H,D,C,0 };
// "Short" font.
static const int _sh_font_img_w = 1024;
static const int _sh_font_img_h = 12;
static const unsigned char _sh_font_img[] = { H,H,F,D,A,255,B,A,Z,B,255,B,Z,255,C,A,255,D,255,B,A,Z,B,255,C,A,255,B,A,Z,B,255,D,Z,255,A,Z,B,A,Z,B,255,B,Z,255,D,B,Z,B,A,Z,B,255,C,B,255,H,H,H,F,D,255,B,Z,B,255,Z,A,Z,255,B,255,Z,A,X,,D,255,B,Z,B,255,Z,A,X,,B,255,B,Z,B,255,Z,A,X,,C,Z,255,B,Z,B,Z,B,255,Z,A,Z,255,B,255,Z,D,255,B,Z,B,255,Z,A,X,,B,255,D,255,B,A,Z,B,255,B,Z,255,C,A,255,D,255,B,A,Z,B,255,E,C,Z,255,A,Z,B,A,Z,B,255,B,Z,255,D,B,Z,B,A,Z,B,255,C,B,255,F,B,255,A,255,C,Z,H,F,D,C,A,255,C,A,255,G,F,E,D,B,A,255,G,D,B,255,C,B,255,B,Z,B,255,Z,A,Z,255,B,255,Z,A,X,,D,255,B,Z,B,255,Z,A,X,,A,Z,B,A,255,B,Z,C,A,Y,B,Y,B,Z,B,Z,B,255,Z,A,Z,255,B,255,Z,D,255,B,Z,B,255,Z,A,X,,B,255,C,A,255,Z,E,D,Z,B,Z,A,255,A,X,,A,Z,Y,C,Z,B,255,B,A,255,B,255,B,A,255,E,A,Y,255,B,255,B,Y,255,A,Z,Y,A,X,,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,D,Z,C,Z,B,A,Y,B,Y,B,255,B,Y,B,Z,255,A,Z,255,B,Y,255,A,Z,Y,B,Y,A,X,,A,Z,Y,B,Z,A,X,,A,Z,B,Z,A,X,,A,255,A,Z,B,Y,B,Z,B,Y,B,Z,255,A,Z,Y,A,X,,A,X,,A,Z,A,X,,Z,A,X,,A,Z,Y,A,Z,255,B,255,B,Z,255,B,255,Z,C,A,255,C,A,255,C,B,Z,C,A,Y,C,Z,B,Z,B,A,255,B,255,B,Z,E,D,255,E,C,Z,255,A,Z,B,Z,255,C,A,255,B,A,Y,D,255,B,Z,B,Z,B,Z,B,Z,B,Z,B,Z,B,Z,Y,A,Z,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,Y,A,Z,A,255,A,Z,B,Z,B,Z,B,Z,B,Z,C,A,Z,B,255,Z,A,X,,A,X,,A,X,,A,X,,A,Z,255,B,Z,255,E,A,255,E,C,Z,B,255,B,X,,A,X,,B,Z,E,C,255,E,D,B,Z,Y,255,C,255,E,255,C,A,Z,Y,Y,X,,A,Z,A,255,A,255,Z,A,Z,B,Z,B,A,255,B,Y,255,B,255,D,C,Z,A,255,Z,A,Z,255,B,A,255,B,Z,A,X,,A,Z,B,Z,B,A,Z,A,X,,A,X,,A,Z,B,A,255,B,Z,B,Y,255,B,255,B,255,A,255,A,255,A,255,A,255,Z,A,X,,A,Z,B,X,,A,Z,B,Z,B,Z,B,X,,B,255,B,A,255,A,X,,A,Z,B,Z,255,Z,255,Z,255,Z,A,255,Z,A,X,,A,X,,A,X,,A,Z,B,A,255,B,255,Z,A,X,,A,Z,A,X,,Z,A,X,,B,Z,A,Z,B,A,255,B,Z,D,B,A,Z,255,A,Z,255,B,Z,255,B,Y,B,Y,B,255,B,Z,255,A,Z,255,D,255,Z,A,Z,B,Z,255,Z,A,Y,B,Z,B,Y,B,Z,255,A,Z,255,B,Z,255,A,Z,Y,A,X,,A,X,,A,Z,A,X,,Z,A,X,,A,Z,Y,B,255,B,255,B,A,255,B,X,,C,255,A,255,A,Z,A,255,Z,A,255,C,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,Z,B,Z,B,Z,B,Z,B,Z,B,A,255,B,Z,B,Z,B,Z,B,X,,A,Y,Z,A,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,Z,A,255,A,Z,255,B,Y,B,Y,B,Y,B,Y,B,Y,B,255,Z,255,A,Y,B,Y,B,Y,B,Y,B,Y,D,C,B,Y,A,Z,255,B,Z,B,Z,B,Z,B,Z,B,Z,C,B,255,X,,Z,A,X,,A,X,,A,X,,A,X,,A,X,,255,A,255,Z,A,X,,B,255,Z,255,X,,B,255,D,C,A,255,C,B,255,Z,A,Z,Y,B,255,B,255,C,A,255,B,A,255,B,Z,B,Y,255,C,Z,Y,C,A,Z,A,X,,B,255,B,Y,255,B,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,Z,A,Z,Y,A,Z,Y,C,B,A,Z,C,B,Z,B,Z,A,X,,255,A,Y,255,A,Z,255,B,255,B,X,,A,Z,255,B,Y,B,255,Z,A,Z,Y,B,255,B,A,255,A,Z,255,B,255,B,X,,X,,Z,255,A,255,Z,A,Z,255,B,255,Z,A,Z,255,B,Z,B,Z,B,255,Z,A,X,,A,X,,Z,A,255,B,Y,255,B,255,B,255,B,A,Z,B,255,D,B,A,255,Z,A,X,,A,Z,B,X,,A,Z,Y,A,Z,Y,A,X,,A,X,,A,Z,B,A,255,B,Y,B,255,B,X,,X,,Z,A,X,,A,X,,A,X,,A,X,,A,Z,255,B,Z,B,255,Z,A,X,,A,Z,A,Z,A,255,B,255,Z,B,Z,A,Z,255,B,255,B,A,Y,A,X,,A,Z,B,Z,B,Z,A,255,A,Z,A,255,A,255,B,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,255,A,255,B,Z,255,B,Y,B,Y,B,Y,B,Z,B,Z,B,Z,B,Z,B,Y,Z,A,255,Z,255,A,255,Z,A,X,,A,X,,A,X,,A,X,,B,255,B,255,Z,A,X,,A,X,,A,X,,A,X,,B,255,B,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,Y,Z,B,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,255,B,255,B,A,255,B,Z,B,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,Z,Y,B,255,Z,A,255,Z,A,X,,A,X,,A,X,,A,X,,A,Z,255,Z,A,255,Z,A,X,,255,A,255,Z,Y,X,,A,X,,E,B,Z,Y,Y,A,X,,A,255,B,255,Z,C,Z,B,A,255,B,255,Z,B,255,D,B,A,Z,B,255,Z,B,255,B,255,B,A,Z,B,Z,B,Z,A,X,,B,Z,A,X,,B,Z,A,Z,C,B,255,B,Y,255,B,255,C,A,255,B,X,,A,X,,A,Z,B,X,,A,Z,B,Z,B,X,,A,X,,B,255,B,255,Z,A,X,,A,Z,B,Z,A,X,,A,255,A,255,Z,A,Z,B,X,,A,X,,B,Z,B,255,B,255,Z,A,X,,A,Z,255,Z,255,X,,B,255,B,255,B,Z,C,255,A,Z,D,B,A,255,Z,A,X,,A,Z,B,X,,A,Z,B,A,255,B,255,Z,A,X,,A,Z,B,A,255,B,255,Z,A,Z,B,X,,X,,Z,A,X,,A,X,,A,X,,A,Z,B,A,Z,B,255,B,255,Z,A,X,,A,X,,Z,A,255,B,255,Z,A,Z,255,B,Z,B,255,B,A,255,C,A,255,B,Z,A,255,A,Z,A,255,Z,A,255,A,255,B,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,Z,B,Z,B,Z,B,Z,B,Z,B,A,255,B,Z,B,Z,B,Z,B,X,,A,255,A,255,A,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,B,255,B,255,Z,A,Z,A,255,A,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,Z,Y,A,Z,B,Z,255,B,Y,B,Y,B,Y,B,Z,B,255,B,A,255,B,Z,B,255,Z,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,C,A,255,Z,A,255,Z,A,X,,A,X,,A,X,,A,X,,A,Z,255,Z,A,255,Z,A,X,,A,X,,255,A,255,Z,A,X,,D,C,Z,C,B,255,Z,A,Z,Y,A,255,A,255,A,X,,C,Z,B,255,D,A,255,C,A,255,B,Z,B,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,Z,A,Z,Y,A,Z,Y,B,Z,A,Z,Y,A,Z,Y,C,A,255,B,A,255,C,Z,B,A,Z,B,Y,255,A,255,Z,A,Z,255,B,Z,255,A,Z,255,B,Y,255,A,Z,B,A,Y,A,X,,A,Z,Y,B,255,B,255,Z,A,Z,Y,A,Z,A,X,,A,255,A,Z,B,255,B,A,255,B,255,Z,A,Z,255,B,Z,B,Y,255,B,255,B,255,A,X,,Z,B,255,B,Y,255,A,Z,255,C,255,Z,255,C,A,Y,Y,255,C,Y,A,Z,255,B,Z,255,B,Y,B,Y,B,255,B,Z,255,A,X,,A,Z,B,A,255,B,255,Z,A,Z,B,Z,A,X,,Z,B,255,B,Y,B,Z,255,A,Z,B,Z,255,B,Z,B,Z,255,B,255,B,X,,A,255,Z,B,Y,A,Z,Y,B,Y,A,Z,B,Z,255,C,A,255,B,A,Y,D,Y,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,A,X,,255,A,Z,255,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,A,Z,Y,B,Y,A,Z,A,255,A,Z,B,Z,B,Z,B,Z,B,Z,C,A,Z,B,Y,255,A,Z,Y,A,Z,Y,A,Z,Y,B,255,B,Y,B,255,Z,255,A,Z,255,B,Y,B,Y,B,Y,B,Y,B,Y,A,X,,Y,A,Y,B,Y,B,Y,B,Y,B,Y,B,255,B,255,B,A,255,B,Z,B,Z,255,A,X,,B,255,B,Z,B,Z,B,Z,B,Z,B,Z,B,255,Z,B,Y,B,Y,B,Y,B,Y,B,Y,A,X,,255,A,Z,255,B,Y,Y,A,255,Z,255,A,255,B,Z,E,C,B,A,255,E,C,B,Z,F,D,C,Z,G,B,Z,G,A,Z,D,255,E,A,255,B,A,Z,E,C,B,A,Z,F,E,C,A,Z,G,B,A,Z,B,Z,E,D,Z,G,A,Z,C,A,Z,C,B,A,Z,B,Z,H,G,E,D,C,B,A,Z,255,D,255,E,A,Z,B,A,Z,E,C,B,Z,255,F,E,C,A,Z,G,E,D,C,B,A,Z,G,A,Y,C,A,Y,D,Z,B,Z,H,H,H,H,H,H,H,H,D,C,0,0 };
// clang-format on
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef X
#undef Y
#undef Z
// clang-format on

// because string.h doesn't always have strnlen()
static int _apg_pixfont_strnlen( const char* str, int maxlen ) {
  int i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
}

#define MASK_FIRST_ONE 128   // 128 or 10000000
#define MASK_FIRST_TWO 192   // 192 or 11000000
#define MASK_FIRST_THREE 224 // 224 or 11100000
#define MASK_FIRST_FOUR 240  // 240 or 11110000
#define MASK_FIRST_FIVE 248  // 248 or 11111000
static uint32_t _utf8_to_cp( const char* mbs, int* sz ) {
  assert( mbs && sz );
  if ( !mbs || !sz ) { return 0; }
  *sz = 0;
  if ( '\0' == mbs[0] ) { return 0; }
  uint8_t first_byte = (uint8_t)mbs[0];
  if ( first_byte < MASK_FIRST_ONE ) {
    *sz = 1;
    return (uint32_t)mbs[0];
  }
  if ( first_byte < MASK_FIRST_THREE ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t part_a     = first_byte << 3;             // shift 110xxxxx to xxxxx000
    uint8_t part_b     = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint32_t codepoint = (uint32_t)part_a << 3;       // 00000000 00000000 00000000 xxxxx000 << 3 = 00000000 00000000 00000xxx xx000000
    codepoint |= (uint32_t)part_b;                    // 00000000 00000000 00000xxx xx000000 | pb = 00000000 00000000 00000xxx xxxxxxxx
    *sz = 2;
    return codepoint;
  }
  if ( first_byte < MASK_FIRST_FOUR ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t third_byte = (uint8_t)mbs[2];
    if ( third_byte < MASK_FIRST_ONE || third_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t part_a = first_byte << 4;             // shift 1110xxxx to xxxx0000
    uint8_t part_b = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_c = third_byte & (uint8_t)0x3F;  // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    (void)part_b;                                 // unused
    (void)part_c;                                 // unused

    uint32_t codepoint = (uint32_t)part_a << 2;
    codepoint          = ( codepoint | (uint32_t)second_byte ) << 6;
    codepoint |= (uint32_t)third_byte;
    *sz = 3;
    return codepoint;
  }
  if ( first_byte < MASK_FIRST_FIVE ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t third_byte = (uint8_t)mbs[2];
    if ( third_byte < MASK_FIRST_ONE || third_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t fourth_byte = (uint8_t)mbs[3];
    if ( fourth_byte < MASK_FIRST_ONE || fourth_byte >= MASK_FIRST_TWO ) { return 0; }
    uint8_t part_a     = first_byte << 4;             // shift 1110xxxx to xxxx0000
    uint8_t part_b     = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_c     = third_byte & (uint8_t)0x3F;  // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_d     = fourth_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint32_t codepoint = (uint32_t)part_a << 1;
    codepoint          = ( codepoint | (uint32_t)part_b ) << 6;
    codepoint          = ( codepoint | (uint32_t)part_c ) << 6;
    codepoint |= (uint32_t)part_d;
    *sz = 4;
    return codepoint;
  }
  return 0;
}

static uint32_t _atlas_index_for_sequence( const char* sequence, int* bytes_processed ) {
  assert( sequence && bytes_processed );

  int len              = _apg_pixfont_strnlen( sequence, APG_PIXFONT_MAX_STRLEN );
  uint32_t atlas_index = '?';
  uint8_t first_byte   = (uint8_t)sequence[0];
  *bytes_processed     = 0;
  if ( first_byte >= ' ' && first_byte <= '~' ) { return (uint32_t)first_byte; }
  if ( len > 1 ) {
    uint32_t codepoint = _utf8_to_cp( sequence, bytes_processed );
    // Punctuation from Latin-1
    switch ( codepoint ) {
    case 0xA1: {
      atlas_index = '~' + 1;
    } break; // Inverted exclamation mark.
    case 0xA9: {
      atlas_index = '~' + 2;
    } break; // Copyright sign.
    case 0xAB: {
      atlas_index = '~' + 3;
    } break; // Left double angle quotation mark.
    case 0xBB: {
      atlas_index = '~' + 4;
    } break; // Right double angle quotation mark.
    case 0xBF: {
      atlas_index = '~' + 5;
    } break; // Inverted question mark.
    default: break;
    }
    // Letters and mathematical operators from Latin-1.
    if ( codepoint >= 0xC0 && codepoint <= 0xFF ) { atlas_index = codepoint - 0xC0 + '~' + 6; }
    // Extras
    if ( codepoint == 0x0152 ) { atlas_index = 196; } // OE
    if ( codepoint == 0x0153 ) { atlas_index = 197; } // oe
    if ( codepoint == 0x01EA ) { atlas_index = 198; } // O ogonek (tail)
    if ( codepoint == 0x01EB ) { atlas_index = 199; } // o ogonek (tail)
  }
  return atlas_index;
}

static int _get_spacing_for_codepoint( uint32_t codepoint, typeface_t typeface ) {
  // Size here includes the blank space pixel following. So a single-column glyph returns 2.

  int default_w = 6; // Default to showing whole glyph cell, as it's easier to debug extra spacing that clipped-off pixels.

  if ( APG_PIXFONT_TYPEFACE_STANDARD == typeface ) {
    if ( '!' == codepoint || 0x27 == codepoint || '.' == codepoint || ':' == codepoint || 'l' == codepoint || '|' == codepoint || 0xA1 == codepoint ) {
      return 2;
    }
    if ( ',' == codepoint || ';' == codepoint || '`' == codepoint ) { return 3; }
    if ( '(' == codepoint || ')' == codepoint || '<' == codepoint || '>' == codepoint || '[' == codepoint || ']' == codepoint || '{' == codepoint || '}' == codepoint ) {
      return 4;
    }
    if ( ' ' == codepoint || '/' == codepoint || '\'' == codepoint ) { return 5; }
  } else if ( APG_PIXFONT_TYPEFACE_SHORT == typeface ) {
    if ( codepoint >= '0' && codepoint <= '9' ) {
      return 4;
    } // Numbers all 3px wide.
      // Upper and lowercase and accented letters mostly 3px wide.
    if ( ( codepoint >= 'a' && codepoint <= 'z' ) || ( codepoint >= 'A' && codepoint <= 'Z' ) || ( codepoint >= 0xC0 && codepoint <= 0xFF ) ) { default_w = 4; }
    if ( '!' == codepoint || 0x27 == codepoint || '.' == codepoint || ':' == codepoint || 'i' == codepoint || 'l' == codepoint || '|' == codepoint || 0xA1 == codepoint ) {
      return 2;
    }
    if ( '(' == codepoint || ')' == codepoint || ',' == codepoint || ';' == codepoint || '[' == codepoint || ']' == codepoint || '`' == codepoint || 'j' == codepoint ) {
      return 3;
    }
    if ( '*' == codepoint || '+' == codepoint || '-' == codepoint || '=' == codepoint || '^' == codepoint || '<' == codepoint || '>' == codepoint ||
         '{' == codepoint || '}' == codepoint || 0xBF == codepoint ) {
      return 4;
    }
    if ( ' ' == codepoint || '%' == codepoint || '&' == codepoint || '/' == codepoint || '?' == codepoint || '@' == codepoint || 'N' == codepoint ||
         0x5C == codepoint || '~' == codepoint || 0xA9 == codepoint || 0xC6 == codepoint || 0xD0 == codepoint || 0xD1 == codepoint || 0xDF == codepoint ) {
      return 5;
    }
    if ( 'M' == codepoint || 'W' == codepoint || 'm' == codepoint || 'w' == codepoint || 0xD8 == codepoint || 0xE6 == codepoint || 0xF8 == codepoint || 0x152 == codepoint ) {
      return 6;
    }
  }
  return default_w;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void apg_pixfont_word_wrap_str( char* str_ptr, int col_max ) {
  if ( !str_ptr || col_max <= 0 ) { return; }

  int len = _apg_pixfont_strnlen( str_ptr, APG_PIXFONT_MAX_STRLEN );
  if ( len < col_max ) { return; } // Entire string fits in one line.

  int word_start = 0, word_n = 0, col = 0;
  for ( int i = 0; i < len; i++ ) {
    if ( col > col_max ) {
      if ( word_start > 0 ) {
        str_ptr[word_start - 1] = '\n'; // Might introduce double line-breaks.
        col                     = word_n;
      }
    }
    if ( isspace( str_ptr[i] ) ) {
      word_n     = 0;
      word_start = i + 1;
      if ( '\n' == str_ptr[i] ) {
        col = 0;
        continue;
      }
    } else {
      word_n++;
    }
    col++;
  }
}

int apg_pixfont_image_size_for_str( const char* ascii_str, int* w, int* h, int thickness, int add_outline, apg_pixfont_style_t style, int col_max, typeface_t typeface ) {
  if ( !ascii_str || !w || !h || thickness < 1 ) { return APG_PIXFONT_FAILURE; }

  *w = *h = 0;

  int typeface_img_h = typeface == APG_PIXFONT_TYPEFACE_STANDARD ? _font_img_h : _sh_font_img_h;

  int len = _apg_pixfont_strnlen( ascii_str, APG_PIXFONT_MAX_STRLEN );
  if ( 0 == len ) { return APG_PIXFONT_FAILURE; }

  int x_cursor = 0, y_cursor = 0, max_x = 0;
  int last_drawn_y_cursor = 0;
  for ( int i = 0, col = 0; i < len; i++ ) {
    if ( '\r' == ascii_str[i] ) { continue; } // Ignore carriage return.
    if ( '\n' == ascii_str[i] ) {
      y_cursor += typeface_img_h;
      x_cursor = col = 0;
      continue;
    }
    if ( col_max > 0 && col >= col_max ) {
      y_cursor += typeface_img_h;
      x_cursor = col = 0;
      if ( ' ' == ascii_str[i] ) { continue; } // Skip spaces after wrap.
    }
    int bytes_read       = 0;
    uint32_t atlas_index = _atlas_index_for_sequence( &ascii_str[i], &bytes_read );
    if ( bytes_read > 1 ) { i += ( bytes_read - 1 ); }
    x_cursor += _get_spacing_for_codepoint( atlas_index, typeface );
    x_cursor = ( style != APG_PIXFONT_STYLE_BOLD ) ? x_cursor : x_cursor + 1;
    max_x    = x_cursor > max_x ? x_cursor : max_x;
    col++;
    last_drawn_y_cursor = y_cursor;
  } // endfor chars in str

  *w = max_x; // Each char is ~6px wide + 1 spacing px.
  *h = last_drawn_y_cursor + typeface_img_h;

  *w = *w * thickness;
  *h = *h * thickness;

  if ( APG_PIXFONT_STYLE_ITALIC == style ) { *w = *w + 7; }
  if ( APG_PIXFONT_STYLE_UNDERLINE == style || APG_PIXFONT_STYLE_STRIKETHROUGH == style ) { *w = *w + thickness; }
  if ( add_outline ) {
    *w = *w + 1;
    *h = *h + 1;
  }
  // make sure size is an even number, to help alignment of image
  if ( *w % 2 != 0 ) { *w = *w + 1; }
  if ( *h % 2 != 0 ) { *h = *h + 1; }

  return APG_PIXFONT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool _is_img_idx_coloured( const unsigned char* image, int idx, int n_channels ) {
  if ( 1 == n_channels && image[idx] ) { return true; }
  if ( 2 == n_channels && image[idx + 1] ) { return true; }
  if ( 3 == n_channels && image[idx] + image[idx + 1] + image[idx + 2] > 0 ) { return true; }
  if ( 4 == n_channels && image[idx + 3] ) { return true; }
  return false;
}

// Outline is dark rather than black so it can match text colour e.g. in a shader multiplication.
// And so that in e.g. a 1-channel image it can be differentiated from the background.
static void _apply_outline( unsigned char* image, int idx, int n_channels ) {
  for ( int c = 0; c < n_channels; c++ ) { image[idx * n_channels + c] = 0x33; }
  if ( 2 == n_channels ) { image[idx * n_channels + 1] = 0xFF; } // Don't set alpha to 0.
  if ( 4 == n_channels ) { image[idx * n_channels + 3] = 0xFF; } // Don't set alpha to 0.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int apg_pixfont_str_into_image(                                       //
  const char* ascii_str,                                              //
  unsigned char* image,                                               //
  int w, int h,                                                       //
  int n_channels,                                                     //
  unsigned char r, unsigned char g, unsigned char b, unsigned char a, //
  int thickness,                                                      //
  int add_outline,                                                    //
  apg_pixfont_style_t style,                                          //
  int col_max,                                                        //
  typeface_t typeface                                                 //
) {
  if ( !ascii_str || !image || n_channels < 1 || n_channels > 4 || thickness < 1 ) { return APG_PIXFONT_FAILURE; }

  int len                           = _apg_pixfont_strnlen( ascii_str, APG_PIXFONT_MAX_STRLEN );
  int x_cursor                      = 0;
  int y_cursor                      = 0;
  int typeface_img_h                = typeface == APG_PIXFONT_TYPEFACE_STANDARD ? _font_img_h : _sh_font_img_h;
  int typeface_img_w                = typeface == APG_PIXFONT_TYPEFACE_STANDARD ? _font_img_w : _sh_font_img_w;
  const unsigned char* typeface_img = APG_PIXFONT_TYPEFACE_STANDARD ? _font_img : _sh_font_img;

  uint8_t colour[4] = { r, g, b, a };
  if ( 2 == n_channels ) { colour[1] = a; } // 2-channel is usually RedAlpha, not RG.

  for ( int i = 0, col = 0; i < len; i++ ) {
    if ( '\n' == ascii_str[i] ) {
      y_cursor += typeface_img_h * thickness;
      x_cursor = col = 0;
      continue;
    }
    // Here separately to avoid double line-breaks if a `\n` was on the char limit. Note no `continue;` statement,
    // so we still print the overlapping char.
    if ( col_max > 0 && col >= col_max ) {
      y_cursor += typeface_img_h * thickness;
      x_cursor = col = 0;
      if ( ' ' == ascii_str[i] ) { continue; } // Skip spaces after wrap.
    }
    if ( '\r' == ascii_str[i] ) { continue; } // Ignore carriage return. Note that space isn't ignored/skipped because we sometimes drawn them e.g. underlines.
    int bytes_read     = 0;
    uint32_t codepoint = _atlas_index_for_sequence( &ascii_str[i], &bytes_read );
    if ( bytes_read > 1 ) { i += ( bytes_read - 1 ); }
    int spacing_px         = _get_spacing_for_codepoint( codepoint, typeface );
    int white_part_spacing = add_outline ? spacing_px - 1 : spacing_px;
    uint32_t atlas_index = codepoint > 32 ? codepoint - 33 : 0; // Strip 'atlas' has no space graphics. Maybe it should. For space this gives a giant number and
                                                                // this works kinda by accident.
    int max_x_offset = 0;
    // For each gyph in the thin strip of letters image.
    for ( int y = 0; y < typeface_img_h; y++ ) {
      for ( int x = 0; x < white_part_spacing; x++ ) {
        int atlas_x       = atlas_index * 6 + x;
        int atlas_y       = y;
        int atlas_img_idx = typeface_img_w * atlas_y + atlas_x;
        // 0 is top of glyph subimage, 10 is the baseline.
        if ( ( codepoint > 32 && typeface_img[atlas_img_idx] > 0x00 ) || //
             ( APG_PIXFONT_STYLE_UNDERLINE == style && y == 12 ) ||      //
             ( APG_PIXFONT_STYLE_STRIKETHROUGH == style && y == 8 )      //
        ) {
          // Fatten if necessary.
          for ( int y_th = 0; y_th < thickness; y_th++ ) {
            for ( int x_th = 0; x_th < thickness; x_th++ ) {
              int image_x  = x_cursor + x * thickness + x_th;
              int image_y  = y_cursor + y * thickness + y_th;
              int x_offset = 0;
              if ( APG_PIXFONT_STYLE_ITALIC == style ) {
                x_offset     = 7 - y / 2; // Bottom left (y 14 and 15) dont move. Every 2 px up from that move 1. Max x is 6 + 7 + outline (14).
                max_x_offset = x_offset > max_x_offset ? x_offset : max_x_offset;
              } else if ( APG_PIXFONT_STYLE_BOLD == style ) {
                x_offset = image_x % 1;
              }

              int out_img_idx = w * image_y + image_x + x_offset;
              if ( image_x >= w || image_y >= h ) { continue; }
              for ( int c = 0; c < n_channels; c++ ) { image[out_img_idx * n_channels + c] = colour[c]; }
              if ( APG_PIXFONT_STYLE_BOLD == style ) {
                for ( int c = 0; c < n_channels; c++ ) { image[( out_img_idx + 1 ) * n_channels + c] = colour[c]; }
                max_x_offset = 1;
              }
              if ( ( APG_PIXFONT_STYLE_UNDERLINE == style && y == 12 ) || ( APG_PIXFONT_STYLE_STRIKETHROUGH == style && y == 8 ) ) { // Already looping over
                                                                                                                                     // thickness so just need
                                                                                                                                     // one offset here.
                int extra_ul_px = add_outline ? thickness : thickness - 1;
                for ( int c = 0; c < n_channels; c++ ) { image[( out_img_idx + extra_ul_px ) * n_channels + c] = colour[c]; }
              } // endif underline.
            } // endfor x thickness.
          } // endfor y thickness.
        } // endif colours.
      } // endfor glyph x.
    } // endfor glyph y.
    x_cursor += spacing_px * thickness; // TODO use actual max x written to above
    x_cursor = ( style == APG_PIXFONT_STYLE_BOLD || style == APG_PIXFONT_STYLE_UNDERLINE ) ? x_cursor + max_x_offset : x_cursor;
    col++;
  } // endfor chars in str

  // NOTE(Anton) this is verbose because i have to do a whole 'nother loop order and y neighbour direction if the image memory is vertically flipped.
  if ( add_outline ) {
    for ( int y = h - 1; y >= 0; y-- ) {
      for ( int x = w - 1; x >= 0; x-- ) {
        if ( _is_img_idx_coloured( image, n_channels * ( w * y + x ), n_channels ) ) { continue; }
        if ( y > 0 ) {
          if ( _is_img_idx_coloured( image, n_channels * ( w * ( y - 1 ) + x ), n_channels ) ) {
            _apply_outline( image, w * y + x, n_channels );
            continue;
          }
        }
        if ( x > 0 ) {
          if ( _is_img_idx_coloured( image, n_channels * ( w * y + ( x - 1 ) ), n_channels ) ) {
            _apply_outline( image, w * y + x, n_channels );
            continue;
          }
        }
        if ( y > 0 && x > 0 ) {
          if ( _is_img_idx_coloured( image, n_channels * ( w * ( y - 1 ) + ( x - 1 ) ), n_channels ) ) {
            _apply_outline( image, w * y + x, n_channels );
            continue;
          }
        }
      } // endforx
    } // endfor y
  } // endfor outline
  return APG_PIXFONT_SUCCESS;
}
