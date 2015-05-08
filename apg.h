//
// apg.h - generic C99 utility functions
// first version Dr Anton Gerdelan, 8 May 2015
//

#pragma once

#include <stdint.h>

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

//
// replacement for the deprecated min/max functions from original C spec
//
uint maxu (int a, int b);
int maxi (int a, int b);
float maxf (float a, float b);
double maxd (double a, double b);
long double maxld (long double a, long double b);
uint minu (int a, int b);
int mini (int a, int b);
float minf (float a, float b);
double mind (double a, double b);
long double minld (long double a, long double b);

//
// generic domain-clamping functions
//
int clampu (int x, int min, int max);
int clampi (int x, int min, int max);
float clampf (float x, float min, float max);
double clampd (double x, double min, double max);
long double clampld (long double x, long double min, long double max);

/****************************************************************************/
/****************************** IMPLEMENTATION ******************************/
/****************************************************************************/

inline uint maxu (int a, int b) { return a > b ? a : b; }
inline int maxi (int a, int b) { return a > b ? a : b; }
inline float maxf (float a, float b) { return a > b ? a : b; }
inline double maxd (double a, double b) { return a > b ? a : b; }
inline long double maxld (long double a, long double b) {
	return a > b ? a : b; }
inline uint minu (int a, int b) { return a > b ? a : b; }
inline int mini (int a, int b) { return a > b ? a : b; }
inline float minf (float a, float b) { return a > b ? a : b; }
inline double mind (double a, double b) { return a > b ? a : b; }
inline long double minld (long double a, long double b) {
	return a > b ? a : b; }

inline int clampu (int x, int min, int max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}

inline int clampi (int x, int min, int max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}

inline float clampf (float x, float min, float max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}

inline double clampd (double x, double min, double max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}

inline long double clampld (long double x, long double min, long double max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}