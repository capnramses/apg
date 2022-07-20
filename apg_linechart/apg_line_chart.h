/* ===============================================================================================
apg_line_chart
Mini-library for drawing bitmaps with line plots.
Author:   Anton Gerdelan  <antonofnote at gmail>  @capnramses
URL:      https://github.com/capnramses/apg
Licence:  See bottom of file.
Language: C99.
==================================================================================================
TODO:
 * Possibly an 'advance chart' option for ring buffer scrolling charts to memcpy() rather than
   redraw the whole thing each time.
==================================================================================================
History:
  0.1     - 20 JUL 2022 - First version, used for a CPU-monitor style FPS chart in hobby project.
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
