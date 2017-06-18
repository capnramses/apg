//
// Interpolation Functions
// Anton Gerdelan
// First v 15 Mar 2015
// Public Domain - no warrenty implied; use at your own risk.
//
// Notes:
//
// nice demos found at: http://easings.net/
//
// typical variables used by 'tweening' notes on the web are:
// t = current time
// b = beginning value
// c = change in value
// d = duration
// functions are named "in" and "out"
// this is non-standard weirdness, so I shan't be using that
//
// better notation:
// x_i = starting 'time' value
// x_f = ending 'time' value
// x = current 'time' value
// y_i = 'start value' (at x = 0.0)
// y_f = 'end value' (at x= 1.0)
// returns y
// functions are named "accel" and "decel" instead of "in" and "out"
//
// internally:
//
// x - x_i == t
// y_i == b
// y_r == c
// x_r == d
// x_n == t / d
//

#ifndef _APG_INTERP_H_
#define _APG_INTERP_H_

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI // not defined in strict c99
#define M_PI 3.14159265358979323846264338327
#define M_PI_2 M_PI / 2.0
#endif
#define HALF_PI M_PI_2

float lerp (float x, float x_i, float x_f, float y_i, float y_f);
float accel_sine (float x, float x_i, float x_f, float y_i, float y_f);
float accel_exp (float x, float x_i, float x_f, float y_i, float y_f);
float bow_string (float x, float x_i, float x_f, float y_i, float y_f);
float decel_sine (float x, float x_i, float x_f, float y_i, float y_f);
float decel_elastic (float x, float x_i, float x_f, float y_i, float y_f);
float decel_bounce (float x, float x_i, float x_f, float y_i, float y_f);

//
// linear interpolation
// y = x
inline float lerp (float x, float x_i, float x_f, float y_i, float y_f) {
	// work out x as factor between 0 and 1
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	// same for y
	float y_r = y_f - y_i;
	// core function
	float y = x_n;
	return y_r * y + y_i;
}

//
// aka "easeInSine"
// accelerating with cosine wave
// gentle accel that smooths out
// y = 1.0 - cos (x * 0.5 * pi)
inline float accel_sine (float x, float x_i, float x_f, float y_i, float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	// returns wave from -1 to 0, hence the "=1.0 - f(x)"
	float y = 1.0f - cosf (x_n * HALF_PI);
	return y_r * y + y_i;
}

//
// aka "easeInExpo"
// like easeInSine but continuing sharper accel
// y = 2^(10 * (x-1))
// e.g. gravity
inline float accel_exp (float x, float x_i, float x_f, float y_i, float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	// powf is from C99
	float y = powf (2.0f, 10.0f * (x_n - 1.0f));
	return y_r * y + y_i;
}

//
// similar to "easeInBack"
// first half of domain is a sine wave from 0.0 to trough -0.5amp to 0.0.
// second half of domain is a basic quadratic from 0.0 to 1.0amp
// x_n < 0.5 -> f(x) = sin(2x*pi)*-0.5 // considers 0.0:0.5 range
// x_n >= 0.5 -> f(x) = x^2 so = 4 * (x-0.5)^2 // considers 0.5:1.0 range
// e.g. bow string
inline float bow_string (float x, float x_i, float x_f, float y_i, float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	float y = 0.0f;
	// pull back part
	if (x_n < 0.5f) {
		y = sinf (2.0f * x_n * M_PI) * -0.5f;
	// accel part
	} else {
		y = 4.0f * (x_n - 0.5f) * (x_n - 0.5f);
	}
	return y_r * y + y_i;
}

//
// aka "easeOutSine"
// decelerating with cosine wave
// f(x) = sin(x * 0.5 * pi)
inline float decel_sine (float x, float x_i, float x_f, float y_i, float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	float y = sinf (x_n * HALF_PI);
	return y_r * y + y_i;
}

//
// similar to "easeOutElastic" but with no lead-up
// starts at y=0, wobbles about 0 (between +y_f and -y_f) diminishing to y=0
// f(x) = (1.0 - x)*sin(x*pi*8);
// e.g. guitar string
inline float decel_elastic (float x, float x_i, float x_f, float y_i,
	float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	// num of peaks and troughs
	float n = 8.0f;
	float y = (1.0f - x_n) * sinf (x_n * M_PI * n);
	return y_r * y + y_i;
}

//
// similar to "easeOutBounce" but with no lead-up
// like decel_elastic but waves always positive
// f(x) = (1.0 - x)*abs(sin(x*pi*8));
// e.g. ball hitting a surface at 1.0
float decel_bounce (float x, float x_i, float x_f, float y_i, float y_f) {
	float x_r = x_f - x_i;
	float x_n = (x - x_i) / x_r;
	float y_r = y_f - y_i;
	// num of peaks and troughs
	float n = 8.0f;
	float y = (1.0f - x_n) * fabs (sinf (x_n * M_PI * n));
	return y_r * y + y_i;
}

//
// aka "easeInOutElastic"
// delayed spring from 0.0 to 1.0 with wobbles

#endif
