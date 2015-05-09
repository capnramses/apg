//
// apg.h - generic C99 utility functions
// first version Dr Anton Gerdelan, 8 May 2015
//

//
// TODO
// * linearise/unlinearise function
// * ascii or unicode fetcher/printer thing
// * string manip/trim

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

//
// replacement for the deprecated min/max functions from original C spec
// was going to have a series of GL-like functions but it was a lot of
// fiddly code/alternatives, so I'm just copying from stb.h here. as much as I
// dislike pre-processor directives, this makes sense. I believe the trick is
// to have all the parentheses. same deal for clamp
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x ,lo, hi) (MIN (hi, MAX (lo, x)))

// C99 removed M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TAU 2.0 * M_PI

// unit conversions
#define DEG2RAD(a) ((a)*(M_PI/180.0))
#define RAD2DEG(a) ((a)*(180.0/M_PI))

//
// shorthand types for byte and bit manipulation (I don't like the _t suffix)
//
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8; // most useful one for byte in memory
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef unsigned int uint;
