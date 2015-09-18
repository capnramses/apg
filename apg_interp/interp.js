//
// Interpolation Functions for Animation
// aka "Tweening" or "Easing"
// Idea was to do something similar to other libs but with better structure
// First version 23 March 2015 by Anton Gerdelan
//

//
// not an interpolation function, but useful when using them
// returns value x clamped between min and max
//
function clamp (x, minx, maxx) {
	if (x < minx) {
		return minx;
	}
	if (x > maxx) {
		return maxx;
	}
	return x;
}

//
// linear interpolation
// y = x
//
function lerp (x, x_i, x_f, y_i, y_f) {
	// work out x as factor between 0 and 1
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	// same for y
	var y_r = y_f - y_i;
	// core function
	var y = x_n;
	return y_r * y + y_i;
}

//
// aka "easeInSine"
// accelerating with cosine wave
// gentle accel that smooths out
// y = 1.0 - cos (x * 0.5 * pi)
//
function accel_sine (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	// returns wave from -1 to 0, hence the "=1.0 - f(x)"
	var y = 1.0 - Math.cos (x_n * Math.PI * 0.5);
	return y_r * y + y_i;
}

//
// aka "easeInExpo"
// like easeInSine but continuing sharper accel
// y = 2^(10 * (x-1))
// e.g. gravity
//
function accel_exp (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	// powf is from C99
	var y = Math.pow (2.0, 10.0 * (x_n - 1.0));
	return y_r * y + y_i;
}

//
// similar to "easeInBack"
// first half of domain is a sine wave from 0.0 to trough -0.5amp to 0.0.
// second half of domain is a basic quadratic from 0.0 to 1.0amp
// x_n < 0.5 -> f(x) = sin(2x*pi)*-0.5 // considers 0.0:0.5 range
// x_n >= 0.5 -> f(x) = x^2 so = 4 * (x-0.5)^2 // considers 0.5:1.0 range
// e.g. bow string
//
function  bow_string (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	var y = 0.0;
	// pull back part
	if (x_n < 0.5) {
		y = Math.sin (2.0 * x_n * Math.PI) * -0.5;
	// accel part
	} else {
		y = 4.0 * (x_n - 0.5) * (x_n - 0.5);
	}
	return y_r * y + y_i;
}

//
// aka "easeOutSine"
// decelerating with cosine wave
// f(x) = sin(x * 0.5 * pi)
//
function decel_sine (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	var y = Math.sin (x_n * Math.PI * 0.5);
	return y_r * y + y_i;
}

//
// similar to "easeOutElastic" but with no lead-up
// starts at y=0, wobbles about 0 (between +y_f and -y_f) diminishing to y=0
// f(x) = (1.0 - x)*sin(x*pi*8);
// e.g. guitar string
//
function decel_elastic (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	// num of peaks and troughs
	var n = 4.0;
	var y = (1.0 - x_n) * Math.sin (x_n * Math.PI * n);
	return y_r * y + y_i;
}

//
// similar to "easeOutBounce" but with no lead-up
// like decel_elastic but waves always positive
// f(x) = (1.0 - x)*abs(sin(x*pi*8));
// e.g. ball hitting a surface at 1.0
//
function decel_bounce (x, x_i, x_f, y_i, y_f) {
	var x_r = x_f - x_i;
	var x_n = (x - x_i) / x_r;
	var y_r = y_f - y_i;
	// num of peaks and troughs
	var n = 4.0;
	var y = (1.0 - x_n) * Math.abs (Math.sin (x_n * Math.PI * n));
	return y_r * y + y_i;
}
