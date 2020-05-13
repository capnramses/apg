/*
OpenGL Debug Drawing Functions
https://github.com/capnramses/opengl_debug_draw
C89 interface, C99 implementation.
Requires OpenGL 3.0 Core or newer, or an older version supporting  ARB_vertex_array_object.
Requires GLSL 1.2.0 or newer.
Anton Gerdelan <antonofnote@gmail.com>
LICENCE - See bottom of this file.

BUILD INSTRUCTIONS
 1. Drop the .c and .h file into your project's source code.
 2. Modify apg_gldb.c to include your OpenGL header. e.g.

    // #include "apg_glcontext.h" // REPLACE THIS INCLUDE WITH YOUR PROJECT'S OPENGL HEADER
    #include <glad/glad.h> // Your header here.

USAGE INSTRUCTIONS
 1. #include "apg_db.h"
 2. Call apg_gldb_init() before adding geometry, but after starting your window and OpenGL context.
 3. Add line-based geometry with the apg_gldb_add_...() functions.
 4. When drawing, update the camera matrix with apg_gldb_update_cam() and then call apg_gldb_draw().
 5. To modify lines call the corresponding apg_gldb_mod_...() functions.
 6. Call apg_gldb_free() when done i.e. at the end of your program.

HISTORY
  0.1 - 11 Jun 2015 - First version.
  0.2 - 16 Jun 2015 - Feature complete v. original spec.
  0.3 - 13 May 2020 - Tidied API and docs. Added _mod_ function for AABB and frustum.

TODO
 - Recreate test/example program.
 - Add test build to CI/CD tests.
*/

#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Memory is reserved to support this many lines at one time. */
#define APG_GLDB_MAX_LINES 10000

/* Reserves memory for drawing and creates GPU resources. */
bool apg_gldb_init();

/* Frees all allocated memory and GPU resources. */
void apg_gldb_free();

/* Creates a new debug line in world coordinate space.
PARAMS
  start_xyz, end_xyz - Arrays of 3 floats for positions of line ends in world space.
  colour_rgba        - Colour to draw the line.
RETURNS the line id.
*/
int apg_gldb_add_line( float* start_xyz, float* end_xyz, float* colour_rgba );

/* Creates a line with a colour that goes from black to coloured in direction of a normal.
PARAMS
  n_xyz       - Array of 3 floats for direction of the normal in world space.
  pos_xyz     - Position for to draw the base of the normal.
  scale       - Length to draw the line representing the normal.
  colour_rgba - Colour to draw the line.
RETURNS the line id.
*/
int apg_gldb_add_normal( float* n_xyz, float* pos_xyz, float scale, float* colour_rgba );

/* Creates 3 lines in a cross with a colour to show a position.
RETURNS the first of 3 contiguous line ids.
*/
int apg_gldb_add_pos( float* pos_xyz, float scale, float* colour_rgba );

/* Draws a box for this axis-aligned bounding box.
PARAMS
  min_xyz     - The minimum bounds (corner) for the box.
  max_xyz     - The maximum bounds (corner) for the box.
  colour_rgba - Colour to render the lines.
RETURNS the first of 12 contiguous line ids
*/
int apg_gldb_add_aabb( float* min_xyz, float* max_xyz, float* colour_rgba );

/* Draws a circle+radius to represent a sphere.
RETURNS the first of 39 line ids
*/
int apg_gldb_add_rad_circle( float* centre_xyz, float radius, float* colour_rgba );

/* Takes 8 xyz corner points for given camera frustum and draws a box whenever apg_gldb_draw() is called.
Most camera code already extracts points from matrices so that is not repeated here.
RETURNS first line's id.
*/
int apg_gldb_add_frustum( float* ftl, float* ftr, float* fbl, float* fbr, float* ntl, float* ntr, float* nbl, float* nbr, float* colour_rgba );

/* Modify or move a line previously added.
RETURNS false if line_id wasn't valid.
*/
bool apg_gldb_mod_line( uint32_t line_id, float* start_xyz, float* end_xyz, float* colour_rgba );

/* Modify or move an axis-aligned bounding box previously added via apg_gldb_add_aabb().
PARAMS
  line_id     - The value returned by apg_gldb_add_aabb().
  min_xyz     - New minimum bounds for the box.
  max_xyz     - New maximum bounds for the box.
  colour_rgba - New colour to render the lines.
RETURNS false if line_id wasn't valid.
TODO(Anton) - the impl of this should modifier the buffer in one op, not individual calls to apg_gldb_mod_line().
*/
bool apg_gldb_mod_aabb( uint32_t line_id, float* min_xyz, float* max_xyz, float* colour_rgba );

bool apg_gldb_mod_frustum( uint32_t line_id, float* ftl, float* ftr, float* fbl, float* fbr, float* ntl, float* ntr, float* nbl, float* nbr, float* colour_rgba );

/* Wipes all the lines for redrawing. Doesn't actually delete the buffer - call apg_gldb_free() do release allocated graphics resources. */
void apg_gldb_reset_lines();

/* Updates the camera matrix so that line points given are defined as being in world coordinate space.
PARAMS
  matrix - A 16 float column-major matrix as 1D array in column order.
*/
void apg_gldb_update_cam( float* PV_mat4 );

/* Draws the lines.
PARAMS
  x_ray - If true then depth testing is disabled.
*/
void apg_gldb_draw( bool x_ray );

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
