/*==============================================================
Stanford PLY mesh file reader/writer
Language: C99
Author:   Anton Gerdelan - @capnramses
Contact:  <antonofnote@gmail.com>
Website:  https://github.com/capnramses/apg - antongerdelan.net/
Licence:  See bottom of this file.

Limitations:

* binary_little_endian and binary_big_endian are not supported.
* Only the first submesh is considered. Others are ignored.
* All vertex components are assumed to be float.
* The following vertex components are supported:
  x,y,z,nx,ny,nz,s,t,red,green,blue,alpha
* Components are optional, but the order is fixed.
  The following is valid:
  x, y, s, t, red, green, blue.
  But the following is not valid:
  s, t, x, y, z
* Edges are ignored.
* Custom material sections are ignored.
* Comments are discarded.
* Only triangular and quad faces are read.
* Quad faces are always converted to triangles.
* Writer supports limited number of mesh formats (see code). Can be added to as required.
* Writer does no space optimisation so is only suitable for debugging small meshes.

History:
01/10/2019 - published to apg repository.
==============================================================*/

#pragma once

typedef enum apg_ply_vertex_comp_format_t {
  APG_PLY_VERTEX_XYZ = 0,
  APG_PLY_VERTEX_XYZ_NXNYNZ_ST_RGBA,
  APG_PLY_VERTEX_XYZ_NXNYNZ_ST_RGB,
  APG_PLY_VERTEX_XYZ_NXNYNZ_RGBA,
  APG_PLY_VERTEX_XYZ_NXNYNZ_RGB,
  APG_PLY_VERTEX_XYZ_NXNYNZ_ST,
  APG_PLY_VERTEX_XYZ_NXNYNZ,
  APG_PLY_VERTEX_XYZ_ST_RGBA,
  APG_PLY_VERTEX_XYZ_ST_RGB,
  APG_PLY_VERTEX_XYZ_ST,
  APG_PLY_VERTEX_XYZ_RGBA,
  APG_PLY_VERTEX_XYZ_RGB,
  APG_PLY_VERTEX_MAX
} apg_ply_vertex_comp_format_t;

typedef struct apg_ply_t {
  apg_ply_vertex_comp_format_t vertex_format;
  float* vertex_data;
  int vertex_count;
  int n_vertex_attribs; // 1 for xyz, 2 for positions and normals...
  int loaded;           // 1 if there were no errors
} apg_ply_t;

/* Read a .ply file given by `filename`.
Memory is allocated to returned `vertex_data` field. You must free this after use.
On success - returned data structure `loaded` field is 1
On failure - `loaded` field is 0
*/
apg_ply_t apg_ply_read( const char* filename );

/* Write a .ply file given by `filename` and filled `ply` struct.
Returns - 1 on success, or 0 on failure
*/
unsigned int apg_ply_write( const char* filename, apg_ply_t ply );

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/
   Copyright 2019 Anton Gerdelan.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------
*/
