/* ===============================================================================================
apg_line_chart
Mini-library for creating quick bitmaps with line plots of a 2D data series.
Author:   Anton Gerdelan  <antonofnote at gmail>  @capnramses
URL:      https://github.com/capnramses/apg
Licence:  See bottom of file.
Language: C99.
==================================================================================================
Compilation Instructions:
 * Drop apg_line_chart.h and .c into your C or C++ project source files.
 * Make sure that your build system or compiler compiles the .c file as C, not as C++.

API Instructions:
 * Call apg_line_chart_init() somewhere in your code to create a new chart of given dimensions.
 * To plot a data series as points call apg_line_chart_plot_points(), providing your array of xy
   data.
 * This library assumes your data memory layout is a 1D or 2D array in the memory order:
    x-value,y-value,x-value,y-value... for n data points.

Test/Example Instructions:
 * To compile the example, assuming you have the whole apg repository, or otherwise have
   stb_image_write.h:
    gcc apg_line_chart.c -D_APG_LINE_CHART_UNIT_TEST -I ../third_party/stb/ -lm
==================================================================================================
TODO:
 * An 'advance chart' option for ring buffer scrolling charts to memcpy() rather than redraw the
   whole thing each time it updates.
 * Possibly try Wu's line draw as an alternative.
 * Possibly allow symbols and/or error bars for plot points. This would require a different sort
   of data range: ~ x-value,y-value,x-error,y-error,...
 * Functions to allow lines/plots/axes to be bigger than 1 pixel, for higher-resolution bitmaps.
 * Maybe add tics to axes. Possibly also an optional apg_pixel_font text or so.
==================================================================================================
History:
  0.1     - 20 JUL 2022 - First version in apg libraries. Pulled from hobby project and tidied up.
==================================================================================================
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct apg_line_chart_params_t {
  int w, h;                         // Pixels dimensions.
  float min_x, max_x, min_y, max_y; // Chart data value bounds on each axis.
} apg_line_chart_params_t;

typedef struct apg_line_chart_t {
  uint8_t* rgb_ptr; // Pixels.
  apg_line_chart_params_t params;
} apg_line_chart_t;

apg_line_chart_t apg_line_chart_init( apg_line_chart_params_t chart_params );

bool apg_line_chart_free( apg_line_chart_t* chart_ptr );

bool apg_line_chart_clear( apg_line_chart_t* chart_ptr );

bool apg_line_chart_plot_lines( apg_line_chart_t* chart_ptr, float* xy_ptr, int n );

bool apg_line_chart_plot_points( apg_line_chart_t* chart_ptr, float* xy_ptr, int n );

bool apg_line_chart_x_axis_draw( apg_line_chart_t* chart_ptr, float y_value );

bool apg_line_chart_y_axis_draw( apg_line_chart_t* chart_ptr, float x_value );

// These values are global, rather than remembered by a particular chart, and apply to all chart drawing operations.
void apg_line_chart_set_background_colour( uint8_t greyscale_value );
void apg_line_chart_set_line_colour( uint8_t r, uint8_t g, uint8_t b );
void apg_line_chart_set_plot_colour( uint8_t r, uint8_t g, uint8_t b );
void apg_line_chart_set_x_axis_colour( uint8_t r, uint8_t g, uint8_t b );
void apg_line_chart_set_y_axis_colour( uint8_t r, uint8_t g, uint8_t b );

#ifdef __cplusplus
}
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
