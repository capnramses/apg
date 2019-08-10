/*
=======================================================================
Interpolation Functions in C99
=======================================================================
Author: Anton Gerdelan @capnramses <antonofnote@gmail.com>
Licence: see bottom of file.
=======================================================================
Version History

* 2015 Mar 15 - First v
* 2019 Aug 10 - added apg_interp prefix to functions

Notes

* nice easing demos found at: http://easings.net/

typical variables used by 'tweening' notes on the web are:
t = current time
b = beginning value
c = change in value
d = duration
functions are named "in" and "out"
this is non-standard weirdness, so I shan't be using that

better notation:
x_i = starting 'time' value
x_f = ending 'time' value
x = current 'time' value
y_i = 'start value' (at x = 0.0)
y_f = 'end value' (at x= 1.0)
returns y
functions are named "accel" and "decel" instead of "in" and "out"

internally:

x - x_i == t
y_i == b
y_r == c
x_r == d
x_n == t / d
=======================================================================
*/

#ifndef _APG_INTERP_H_
#define _APG_INTERP_H_

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI /* not defined in strict c99 */
#define M_PI 3.14159265358979323846264338327
#define M_PI_2 M_PI / 2.0
#endif
#define HALF_PI M_PI_2

float apg_interp_lerp( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_accel_sine( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_accel_exp( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_bow_string( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_decel_sine( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_decel_elastic( float x, float x_i, float x_f, float y_i, float y_f );
float apg_interp_decel_bounce( float x, float x_i, float x_f, float y_i, float y_f );

/* linear interpolation
y = x
*/
inline float apg_interp_lerp( float x, float x_i, float x_f, float y_i, float y_f ) {
  /* work out x as factor between 0 and 1 */
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  /* same for y  */
  float y_r = y_f - y_i;
  /* core function */
  float y = x_n;
  return y_r * y + y_i;
}

/* aka "easeInSine"
accelerating with cosine wave
gentle accel that smooths out
y = 1.0 - cos (x * 0.5 * pi)
*/
inline float apg_interp_accel_sine( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  /* returns wave from -1 to 0, hence the "=1.0 - f(x)" */
  float y = 1.0f - cosf( x_n * HALF_PI );
  return y_r * y + y_i;
}

/* aka "easeInExpo"
like easeInSine but continuing sharper accel
y = 2^(10 * (x-1))
e.g. gravity */
inline float apg_interp_accel_exp( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  /* powf is from C99 */
  float y = powf( 2.0f, 10.0f * ( x_n - 1.0f ) );
  return y_r * y + y_i;
}

/* similar to "easeInBack"
first half of domain is a sine wave from 0.0 to trough -0.5amp to 0.0.
second half of domain is a basic quadratic from 0.0 to 1.0amp
x_n < 0.5 -> f(x) = sin(2x*pi)*-0.5 // considers 0.0:0.5 range
x_n >= 0.5 -> f(x) = x^2 so = 4 * (x-0.5)^2 // considers 0.5:1.0 range
e.g. bow string
*/
inline float apg_interp_bow_string( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  float y   = 0.0f;
  // pull back part
  if ( x_n < 0.5f ) {
    y = sinf( 2.0f * x_n * M_PI ) * -0.5f;
    // accel part
  } else {
    y = 4.0f * ( x_n - 0.5f ) * ( x_n - 0.5f );
  }
  return y_r * y + y_i;
}

/* aka "easeOutSine"
decelerating with cosine wave
f(x) = sin(x * 0.5 * pi)
*/
inline float apg_interp_decel_sine( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  float y   = sinf( x_n * HALF_PI );
  return y_r * y + y_i;
}

/* similar to "easeOutElastic" but with no lead-up
starts at y=0, wobbles about 0 (between +y_f and -y_f) diminishing to y=0
f(x) = (1.0 - x)*sin(x*pi*8);
e.g. guitar string
*/
inline float apg_interp_decel_elastic( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  /* num of peaks and troughs */
  float n = 8.0f;
  float y = ( 1.0f - x_n ) * sinf( x_n * M_PI * n );
  return y_r * y + y_i;
}

/* similar to "easeOutBounce" but with no lead-up
like decel_elastic but waves always positive
f(x) = (1.0 - x)*abs(sin(x*pi*8));
e.g. ball hitting a surface at 1.0
*/
inline float apg_interp_decel_bounce( float x, float x_i, float x_f, float y_i, float y_f ) {
  float x_r = x_f - x_i;
  float x_n = ( x - x_i ) / x_r;
  float y_r = y_f - y_i;
  /* num of peaks and troughs */
  float n = 8.0f;
  float y = ( 1.0f - x_n ) * fabs( sinf( x_n * M_PI * n ) );
  return y_r * y + y_i;
}

/* aka "easeInOutElastic"
delayed spring from 0.0 to 1.0 with wobbles
*/

#endif

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
>
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
>
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------
*/
